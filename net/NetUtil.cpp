/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config.h>

#include "NetUtil.hpp"
#include <common/Util.hpp>

#include "Socket.hpp"
#if ENABLE_SSL && !MOBILEAPP
#include "SslSocket.hpp"
#endif

#include <Poco/Exception.h>
#include <Poco/Net/DNS.h>
#include <Poco/Net/NetworkInterface.h>

#include <netdb.h>

#include <Poco/Net/SocketAddress.h>

namespace net
{

#if !MOBILEAPP

std::string resolveHostAddress(const std::string& targetHost)
{
    try
    {
        return Poco::Net::DNS::resolveOne(targetHost).toString();
    }
    catch (const Poco::Exception& exc)
    {
        LOG_WRN("Poco::Net::DNS::resolveOne(\"" << targetHost
                                                << "\") failed: " << exc.displayText());
        try
        {
            return Poco::Net::IPAddress(targetHost).toString();
        }
        catch (const Poco::Exception& exc1)
        {
            LOG_WRN("Poco::Net::IPAddress(\"" << targetHost
                                              << "\") failed: " << exc1.displayText());
        }
    }

    return targetHost;
}

bool isLocalhost(const std::string& targetHost)
{
    const std::string targetAddress = resolveHostAddress(targetHost);

    const Poco::Net::NetworkInterface::NetworkInterfaceList list =
        Poco::Net::NetworkInterface::list(true, true);
    for (const auto& netif : list)
    {
        std::string address = netif.address().toString();
        address = address.substr(0, address.find('%', 0));
        if (address == targetAddress)
        {
            LOG_TRC("Host [" << targetHost << "] is on the same host as the client: \""
                             << targetAddress << "\".");
            return true;
        }
    }

    LOG_TRC("Host [" << targetHost << "] is not on the same host as the client: \"" << targetAddress
                     << "\".");
    return false;
}

#endif //!MOBILEAPP

std::shared_ptr<StreamSocket>
connect(const std::string& host, const std::string& port, const bool isSSL,
        const std::shared_ptr<ProtocolHandlerInterface>& protocolHandler)
{
    std::shared_ptr<StreamSocket> socket;

    if (host.empty() || port.empty())
    {
        LOG_ERR("Invalid host/port " << host << ':' << port);
        return socket;
    }

    LOG_DBG("Connecting to " << host << ':' << port << " (" << (isSSL ? "SSL)" : "Unencrypted)"));

#if !ENABLE_SSL
    if (isSSL)
    {
        LOG_ERR("Error: isSSL socket requested but SSL is not compiled in.");
        return socket;
    }
#endif

    // FIXME: store the address?
    struct addrinfo* ainfo = nullptr;
    struct addrinfo hints;
    std::memset(&hints, 0, sizeof(hints));
    const int rc = getaddrinfo(host.c_str(), port.c_str(), &hints, &ainfo);

    if (!rc && ainfo)
    {
        for (struct addrinfo* ai = ainfo; ai; ai = ai->ai_next)
        {
            std::string canonicalName;
            if (ai->ai_canonname)
                canonicalName = ai->ai_canonname;

            if (ai->ai_addrlen && ai->ai_addr)
            {
                int fd = ::socket(ai->ai_addr->sa_family, SOCK_STREAM | SOCK_NONBLOCK, 0);
                if (fd < 0)
                {
                    LOG_SYS("Failed to create socket");
                    continue;
                }

                int res = ::connect(fd, ai->ai_addr, ai->ai_addrlen);
                if (fd < 0 || (res < 0 && errno != EINPROGRESS))
                {
                    LOG_SYS("Failed to connect to " << host);
                    ::close(fd);
                }
                else
                {
#if ENABLE_SSL
                    if (isSSL)
                        socket = StreamSocket::create<SslStreamSocket>(host, fd, true, protocolHandler);
#endif
                    if (!socket && !isSSL)
                        socket = StreamSocket::create<StreamSocket>(host, fd, true, protocolHandler);

                    if (socket)
                    {
                        LOG_DBG('#' << fd << " New socket connected to " << host << ':' << port
                                    << " (" << (isSSL ? "SSL)" : "Unencrypted)"));
                        break;
                    }

                    LOG_ERR("Failed to allocate socket for client websocket " << host);
                    ::close(fd);
                    break;
                }
            }
        }

        freeaddrinfo(ainfo);
    }
    else
        LOG_SYS("Failed to lookup host [" << host << "]. Skipping");

    return socket;
}

std::shared_ptr<StreamSocket>
connect(std::string uri, const std::shared_ptr<ProtocolHandlerInterface>& protocolHandler)
{
    std::string scheme;
    std::string host;
    std::string port;
    if (!parseUri(std::move(uri), scheme, host, port))
    {
        return nullptr;
    }

    scheme = Util::toLower(std::move(scheme));
    const bool isSsl = scheme == "https://" || scheme == "wss://";

    return connect(host, port, isSsl, protocolHandler);
}

bool parseUri(std::string uri, std::string& scheme, std::string& host, std::string& port,
              std::string& url)
{
    const auto itScheme = uri.find("://");
    if (itScheme != uri.npos)
    {
        scheme = uri.substr(0, itScheme + 3); // Include the last slash.
        uri = uri.substr(scheme.size()); // Remove the scheme.
    }
    else
    {
        // No scheme.
        scheme.clear();
    }

    const auto itUrl = uri.find('/');
    if (itUrl != uri.npos)
    {
        url = uri.substr(itUrl); // Including the first foreslash.
        uri = uri.substr(0, itUrl);
    }
    else
    {
        url.clear();
    }

    const auto itPort = uri.find(':');
    if (itPort != uri.npos)
    {
        host = uri.substr(0, itPort);
        port = uri.substr(itPort + 1); // Skip the colon.
    }
    else
    {
        // No port, just hostname.
        host = uri;
        port.clear();
    }

    return !host.empty();
}

} // namespace net