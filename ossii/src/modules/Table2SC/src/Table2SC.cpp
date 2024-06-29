/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the OxOffice Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <OxOOL/OxOOL.h>
#include <OxOOL/Logger.h>
#include <OxOOL/Module/Base.h>
#include <OxOOL/HttpHelper.h>
#include <OxOOL/ModuleManager.h>
#include <OxOOL/ConvertBroker.h>

#include <Poco/String.h>
#include <Poco/Path.h>
#include <Poco/File.h>
#include <Poco/TemporaryFile.h>
#include <Poco/FileStream.h>
#include <Poco/MemoryStream.h>
#include <Poco/StreamCopier.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTMLForm.h>

#include <net/Socket.hpp>

class Table2SC : public OxOOL::Module::Base
{
public:
    Table2SC()
    {
    }

    ~Table2SC()
    {
    }

    void initialize() override
    {
        // 依據 config 初始化 loger
        mpLog = std::make_shared<OxOOL::Logger>(getConfig());
    }

    void handleRequest(const Poco::Net::HTTPRequest& request,
                       const std::shared_ptr<StreamSocket>& socket) override
    {
        // 紀錄來源 IP
        const std::string sourceIP = socket->isLocal() ? "127.0.0.1" : socket->clientAddress();

        mpLog->INF("Received a request from " + socket->clientAddress() + ' ' +
                   request.getMethod() + ' ' + request.getURI(), OXLOG_PROG);

        // 只是 HEAD 的話，回應 200 OK 的 http header
        if (OxOOL::HttpHelper::isHEAD(request))
        {
            OxOOL::HttpHelper::sendResponseAndShutdown(socket);
            return;
        }
        // 不是 POST 的話，回應
        else if (!OxOOL::HttpHelper::isPOST(request))
        {
            mpLog->ERR("No HTTP POST method is used.", OXLOG_PROG);
            OxOOL::HttpHelper::sendErrorAndShutdown(
                Poco::Net::HTTPResponse::HTTP_METHOD_NOT_ALLOWED, socket);
            return;
        }

        // 讀取 HTTML Form.
        Poco::MemoryInputStream message(&socket->getInBuffer()[0],
                                        socket->getInBuffer().size());
        const Poco::Net::HTMLForm form(request, message);

        const std::string& title = form.has("title") ? form.get("title") : "noname";
        const std::string& content = form.has("content") ? form.get("content") : "";
        const std::string& toFormat = form.has("format") ? form.get("format") : "ods";

        std::string htmlTemplate = MULTILINE_STRING(
            <!doctype html>
            <html>
                <head>
                    <meta charset="utf-8">
                    <title>%TITLE%</title>
                </head>
                <body>
                    %CONTENT%
                </body>
            </html>
        );

        // 替換內容
        Poco::replaceInPlace(htmlTemplate, std::string("%TITLE%"), title);
        Poco::replaceInPlace(htmlTemplate, std::string("%CONTENT%"), content);

        // 製作暫存路徑
        Poco::Path tmpPath = Poco::Path::forDirectory(Poco::TemporaryFile::tempName());
        // 建立暫存目錄
        Poco::File(tmpPath).createDirectories();

        tmpPath.append(title + ".xls");
        tmpPath.makeFile();

        // 製作暫存檔
        const std::string tmpFile = tmpPath.toString();
        mpLog->INF("Create temporary file: " + tmpFile, OXLOG_PROG);
        Poco::FileOutputStream outputStream(tmpFile, std::ios::binary|std::ios::trunc);
        outputStream << htmlTemplate;
        outputStream.close();

        // 取得轉檔用的 Broker
        auto docBroker = OxOOL::ConvertBroker::create(tmpFile, toFormat);

        // 檔案載入完畢後，觸發這理，執行後續處理
        docBroker->loadedCallback([=]() {
            // 全選
            docBroker->sendMessageToKit("uno .uno:SelectAll");
            // 最佳化欄位寬度，額外增加 0.2 公分
            docBroker->sendMessageToKit("uno .uno:SetOptimalColumnWidth?aExtraWidth:short=200");
            // 另存檔案，並傳給 client
            docBroker->saveAsDocument();
            // 紀錄成功訊息
            mpLog->INF("Convert to '" + toFormat + "' succeeded.", OXLOG_PROG);
        });

        // 非唯讀模式載入檔案
        mpLog->INF("Load file: " + tmpFile, OXLOG_PROG);
        if (!docBroker->loadDocument(socket))
            mpLog->ERR("Failed to create Client Session on docKey [" + docBroker->getDocKey() + "].", OXLOG_PROG);
    }

    //
    void handleAdminMessage(const OxOOL::SendTextMessageFn& sendTextMessage,
                            const StringVector& tokens) override
    {
        // 傳回最新的紀錄
        if (tokens.equals(0, "getLog"))
        {
            // 檢查是否記錄到日誌檔
            if (mpLog->isFileChannel())
            {
                std::ostringstream outStream;
                Poco::FileInputStream inStream(mpLog->logFilePath());
                Poco::StreamCopier::copyStream(inStream, outStream);
                sendAdminTextFrame(sendTextMessage, "log " + outStream.str());
            }
            else if (mpLog->isSyslogChannel())
                sendAdminTextFrame(sendTextMessage, "log Log output to syslog, please contact the administrator.");
            else
                sendAdminTextFrame(sendTextMessage, "log Log output to the console screen.");
        }
    }

private:
    OxOOL::Logger::Ptr mpLog;
};

OXOOL_MODULE_EXPORT(Table2SC);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
