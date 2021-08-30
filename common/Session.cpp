/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config.h>

#include "Session.hpp"

#include <sys/types.h>
#include <ftw.h>
#include <utime.h>

#include <cassert>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <mutex>
#include <set>

#include <Poco/Exception.h>
#include <Poco/Path.h>
#include <Poco/String.h>
#include <Poco/URI.h>

#include "Common.hpp"
#include "Protocol.hpp"
#include "Log.hpp"
#include <TileCache.hpp>
#include "Util.hpp"
#include "Unit.hpp"
#include "JsonUtil.hpp"

using namespace LOOLProtocol;

using Poco::Exception;
using std::size_t;

Session::Session(const std::shared_ptr<ProtocolHandlerInterface> &protocol,
                 const std::string& name, const std::string& id, bool readOnly) :
    MessageHandlerInterface(protocol),
    _id(id),
    _name(name),
    _disconnected(false),
    _isActive(true),
    _lastActivityTime(std::chrono::steady_clock::now()),
    _isCloseFrame(false),
    _isReadOnly(readOnly),
    _isAllowChangeComments(false),
    _docPassword(""),
    _haveDocPassword(false),
    _isDocPasswordProtected(false),
    _watermarkWhenEditing(false),
    _watermarkWhenPrinting(false)
{
}

Session::~Session()
{
}

bool Session::sendTextFrame(const char* buffer, const int length)
{
    if (!_protocol)
    {
        LOG_TRC("ERR - missing protocol " << getName() << ": Send: [" << getAbbreviatedMessage(buffer, length) << "].");
        return false;
    }

    LOG_TRC(getName() << ": Send: [" << getAbbreviatedMessage(buffer, length) << "].");
    return _protocol->sendTextMessage(buffer, length) >= length;
}

bool Session::sendBinaryFrame(const char *buffer, int length)
{
    if (!_protocol)
    {
        LOG_TRC("ERR - missing protocol " << getName() << ": Send: " << std::to_string(length) << " binary bytes.");
        return false;
    }

    LOG_TRC(getName() << ": Send: " << std::to_string(length) << " binary bytes.");
    return _protocol->sendBinaryMessage(buffer, length) >= length;
}

void Session::parseDocOptions(const StringVector& tokens, int& part, std::string& timestamp, std::string& doctemplate)
{
    // First token is the "load" command itself.
    size_t offset = 1;
    if (tokens.size() > 2 && tokens[1].find("part=") == 0)
    {
        getTokenInteger(tokens[1], "part", part);
        ++offset;
    }

    for (size_t i = offset; i < tokens.size(); ++i)
    {
        std::string name;
        std::string value;
        if (!LOOLProtocol::parseNameValuePair(tokens[i], name, value))
        {
            LOG_WRN("Unexpected doc options token [" << tokens[i] << "]. Skipping.");
            continue;
        }

        if (name == "url")
        {
            _docURL = value;
            ++offset;
        }
        else if (name == "jail")
        {
            _jailedFilePath = value;
            ++offset;
        }
        else if (name == "xjail")
        {
            _jailedFilePathAnonym = value;
            ++offset;
        }
        else if (name == "authorid")
        {
            Poco::URI::decode(value, _userId);
            ++offset;
        }
        else if (name == "xauthorid")
        {
            Poco::URI::decode(value, _userIdAnonym);
            ++offset;
        }
        else if (name == "author")
        {
            Poco::URI::decode(value, _userName);
            ++offset;
        }
        else if (name == "xauthor")
        {
            Poco::URI::decode(value, _userNameAnonym);
            ++offset;
        }
        else if (name == "authorextrainfo")
        {
            Poco::URI::decode(value, _userExtraInfo);
            ++offset;
        }
        else if (name == "readonly")
        {
            _isReadOnly = value != "0";
            ++offset;
        }
        else if (name == "password")
        {
            Poco::URI::decode(value, _docPassword);
            _haveDocPassword = true;
            ++offset;
        }
        else if (name == "lang")
        {
            _lang = value;
            ++offset;
        }
        else if (name == "watermarkWhenEditing")
        {
            _watermarkWhenEditing = value!= "0";
            ++offset;
        }
        else if (name == "watermarkWhenPrinting")
        {
            _watermarkWhenPrinting = value!= "0";
            ++offset;
        }
        else if (name == "watermarkText")
        {
            Poco::URI::decode(value, _watermarkText);
            ++offset;
        }
        else if (name == "watermarkOpacity")
        {
            _watermarkOpacity = std::stod(value);
            ++offset;
        }
        else if (name == "watermarkFont")
        {
            std::string decodeFont;
            Poco::URI::decode(value, decodeFont);
            Poco::JSON::Object::Ptr jsonObj;
            if (JsonUtil::parseJSON(decodeFont, jsonObj))
            {
                for (auto it = jsonObj->begin() ; it != jsonObj->end() ; it++)
                {
                    _watermarkFont.set(it->first, it->second);
                }
            }
            ++offset;
        }
        else if (name == "clientAddr")
        {
            _clientAddr = value;
            ++offset;
        }
        else if (name == "timezone")
        {
            _timezone = value;
            ++offset;
        }
        else if (name == "timezoneOffset")
        {
            _timezoneOffset = std::stol(value);
            ++offset;
        }
        else if (name == "timestamp")
        {
            timestamp = value;
            ++offset;
        }
        else if (name == "template")
        {
            doctemplate = value;
            ++offset;
        }
        else if (name == "deviceFormFactor")
        {
            _deviceFormFactor = value;
            ++offset;
        }
    }

    Util::mapAnonymized(_userId, _userIdAnonym);
    Util::mapAnonymized(_userName, _userNameAnonym);
    Util::mapAnonymized(_jailedFilePath, _jailedFilePathAnonym);

    if (tokens.size() > offset)
    {
        if (getTokenString(tokens[offset], "options", _docOptions))
        {
            if (tokens.size() > offset + 1)
                _docOptions += tokens.cat(' ', offset + 1);
        }
    }
}

std::string Session::getConvertedWatermarkText()
{
    std::string retString = getWatermarkText();

    // 使用者 ID ${id}
    Poco::replaceInPlace(retString, std::string("${id}"), getUserId());
    // 使用者名稱 ${name}
    Poco::replaceInPlace(retString, std::string("${name}"), getUserName());
    // 使用者時區 ${timezone}
    Poco::replaceInPlace(retString, std::string("${timezone}"), getTimezone());

    // 使用者 IP ${ip}
    std::string ip = getClientAddr();
    if (Util::startsWith(ip, "::ffff:"))
    {
        ip = ip.substr(7);
    }
    else if (ip == "::1")
    {
        ip = "127.0.0.1";
    }
    Poco::replaceInPlace(retString, std::string("${ip}"), ip);

    // 系統時間加上客戶端時區偏移值，就是客戶端目前時間
    Poco::DateTime clientDateTime(Poco::Timestamp() + Poco::Timespan(getTimezoneOffset() * 60 * -1, 0));

    // 日期 ${yyyy-mm-dd}
    Poco::replaceInPlace(retString, std::string("${yyyy-mm-dd}"),
                         Poco::DateTimeFormatter::format(clientDateTime, "%Y-%n-%e"));
    // 日期 ${mm-dd-yyyy}
    Poco::replaceInPlace(retString, std::string("${mm-dd-yyyy}"),
                         Poco::DateTimeFormatter::format(clientDateTime, "%n-%e-%Y"));
    // 日期 ${dd-mm-yyyy}
    Poco::replaceInPlace(retString, std::string("${dd/mm/yyyy}"),
                         Poco::DateTimeFormatter::format(clientDateTime, "%e-%n-%Y"));

    // 日期 ${yyyy/mm/dd}
    Poco::replaceInPlace(retString, std::string("${yyyy/mm/dd}"),
                         Poco::DateTimeFormatter::format(clientDateTime, "%Y/%n/%e"));
    // 日期 ${mm/dd/yyyy}
    Poco::replaceInPlace(retString, std::string("${mm/dd/yyyy}"),
                         Poco::DateTimeFormatter::format(clientDateTime, "%n/%e/%Y"));
    // 日期 ${dd/mm/yyyy}
    Poco::replaceInPlace(retString, std::string("${dd/mm/yyyy}"),
                         Poco::DateTimeFormatter::format(clientDateTime, "%e/%n/%Y"));

    // 日期 ${yyyy.mm.dd}
    Poco::replaceInPlace(retString, std::string("${yyyy.mm.dd}"),
                         Poco::DateTimeFormatter::format(clientDateTime, "%Y.%n.%e"));
    // 日期 ${mm.dd.yyyy}
    Poco::replaceInPlace(retString, std::string("${mm.dd.yyyy}"),
                         Poco::DateTimeFormatter::format(clientDateTime, "%n.%e.%Y"));
    // 日期 ${dd.mm.yyyy}
    Poco::replaceInPlace(retString, std::string("${dd.mm.yyyy}"),
                         Poco::DateTimeFormatter::format(clientDateTime, "%e.%n.%Y"));

    // 24小時格式時間 ${time}
    Poco::replaceInPlace(retString, std::string("${time}"),
                         Poco::DateTimeFormatter::format(clientDateTime, "%H:%M"));
    // 12小時格式時間 ${ampm}
    Poco::replaceInPlace(retString, std::string("${ampm}"),
                         Poco::DateTimeFormatter::format(clientDateTime, "%h:%M %a"));

    return retString;
}

void Session::disconnect()
{
    if (!_disconnected)
    {
        _disconnected = true;
        shutdown();
    }
}

void Session::shutdown(bool goingAway, const std::string& statusMessage)
{
    LOG_TRC("Shutting down WS [" << getName() << "] " <<
            (goingAway ? "going" : "normal") <<
            " and reason [" << statusMessage << "].");

    // See protocol.txt for this application-level close frame.
    if (_protocol)
    {
        // skip the queue; FIXME: should we flush SessionClient's queue ?
        std::string closeMsg = "close: " + statusMessage;
        _protocol->sendTextMessage(closeMsg.c_str(), closeMsg.size());
        _protocol->shutdown(goingAway, statusMessage);
    }
}

void Session::handleMessage(const std::vector<char> &data)
{
    try
    {
        std::unique_ptr< std::vector<char> > replace;
        if (!Util::isFuzzing() && UnitBase::get().filterSessionInput(this, &data[0], data.size(), replace))
        {
            if (!replace || replace->empty())
                _handleInput(replace->data(), replace->size());
            return;
        }

        if (!data.empty())
            _handleInput(&data[0], data.size());
    }
    catch (const Exception& exc)
    {
        LOG_ERR("Session::handleInput: Exception while handling [" <<
                getAbbreviatedMessage(data) <<
                "] in " << getName() << ": " << exc.displayText() <<
                (exc.nested() ? " (" + exc.nested()->displayText() + ')' : ""));
    }
    catch (const std::exception& exc)
    {
        LOG_ERR("Session::handleInput: Exception while handling [" <<
                getAbbreviatedMessage(data) << "]: " << exc.what());
    }
}

void Session::getIOStats(uint64_t &sent, uint64_t &recv)
{
    if (!_protocol)
    {
        LOG_TRC("ERR - missing protocol " << getName() << ": Get IO stats.");
        sent = 0; recv = 0;
        return;
    }

    _protocol->getIOStats(sent, recv);
}

void Session::dumpState(std::ostream& os)
{
    os << "\t\tid: " << _id
       << "\n\t\tname: " << _name
       << "\n\t\tdisconnected: " << _disconnected
       << "\n\t\tisActive: " << _isActive
       << "\n\t\tisCloseFrame: " << _isCloseFrame
       << "\n\t\tisReadOnly: " << _isReadOnly
       << "\n\t\tdocURL: " << _docURL
       << "\n\t\tjailedFilePath: " << _jailedFilePath
       << "\n\t\tdocPwd: " << _docPassword
       << "\n\t\thaveDocPwd: " << _haveDocPassword
       << "\n\t\tisDocPwdProtected: " << _isDocPasswordProtected
       << "\n\t\tDocOptions: " << _docOptions
       << "\n\t\tuserId: " << _userId
       << "\n\t\tuserName: " << _userName
       << "\n\t\tlang: " << _lang
       << '\n';
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
