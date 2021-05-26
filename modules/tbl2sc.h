#ifndef __TBL2SC_H__
#define __TBL2SC_H__
#include "config.h"
#include "Socket.hpp"
#include <sys/wait.h>
#include "oxoolmodule.h"
#include <memory>

#include <Poco/MemoryStream.h>
#include <Poco/Net/HTMLForm.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Process.h>
#include <Poco/AutoPtr.h>
#include <Poco/FormattingChannel.h>
#include <Poco/Util/XMLConfiguration.h>

using Poco::Process;
using Poco::Net::HTTPRequest;
using Poco::MemoryInputStream;

class Tbl2SC : public oxoolmodule
{
public:
    void handleRequest(std::weak_ptr<StreamSocket>, MemoryInputStream&, HTTPRequest&, SocketDisposition&);
    std::string handleAdmin(std::string);
    std::string ConfigFile;
    Poco::Util::XMLConfiguration *xml_config;
    Tbl2SC();
    virtual ~Tbl2SC();

    virtual bool isTbl2SCUri(std::string);
    virtual void doTbl2SC(const Poco::Net::HTTPRequest&,
            Poco::MemoryInputStream&,
            std::weak_ptr<StreamSocket>);
    virtual void setProgPath(std::string path)
    {
        loPath = path + "/program";
    }

private:
    // logger
    Poco::AutoPtr<Poco::FormattingChannel> channel;
    void setLogPath();
    // Add for logging database
    std::string reqClientAddress;
    void log(bool success, std::string title, std::string e_msg);
    std::chrono::steady_clock::time_point startStamp;

    std::string loPath;  // soffice program path

    bool validate(const Poco::Net::HTMLForm&,
                  std::weak_ptr<StreamSocket>);
    std::string outputODF(const std::string, const std::string);
    bool isUnoCompleted = false;

    static void ViewCallback(const int, const char*, void*);
};
#endif
