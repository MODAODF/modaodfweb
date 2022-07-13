#ifndef __templaterepo_H__
#define __templaterepo_H__
#include "config.h"
#include "Socket.hpp"
#include "oxoolmodule.h"
#include <memory>

#include <Poco/Tuple.h>
#include <Poco/FileStream.h>
#include <Poco/Net/HTMLForm.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/MemoryStream.h>
#include <Poco/URI.h>
#include <Poco/Process.h>
#include <Poco/AutoPtr.h>
#include <Poco/Util/LayeredConfiguration.h>
#include <Poco/Util/XMLConfiguration.h>
#include <Poco/JSON/Object.h>
#include <Poco/FormattingChannel.h>

using Poco::MemoryInputStream;
using Poco::Path;
using Poco::Process;
using Poco::URI;
using Poco::JSON::Object;
using Poco::Net::HTMLForm;
using Poco::Net::HTTPRequest;
using Poco::Net::NameValueCollection;

class TemplateRepo : public oxoolmodule
{
public:
    void handleRequest(std::weak_ptr<StreamSocket>, MemoryInputStream &, HTTPRequest &, SocketDisposition &);
    std::string handleAdmin(std::string);
    std::string getHTMLFile(std::string);
    void doTemplateRepo(std::weak_ptr<StreamSocket>, MemoryInputStream &, HTTPRequest &);
    TemplateRepo();
    virtual ~TemplateRepo();

    void getInfo(std::weak_ptr<StreamSocket>);
    void syncTemplates(std::weak_ptr<StreamSocket>, const Poco::Net::HTTPRequest &, Poco::MemoryInputStream &);
    void downloadAllTemplates(std::weak_ptr<StreamSocket>);
    std::string makeApiJson(std::string,
                            bool anotherJson = false,
                            bool yaml = false,
                            bool showHead = true);
    bool isTemplateRepoHelpUri(std::string);
    void handleAPIHelp(const Poco::Net::HTTPRequest &, std::weak_ptr<StreamSocket>, Poco::MemoryInputStream &);
    std::string zip_DIFF_FILE(Object::Ptr object);
    std::string zip_ALL_FILE();
    Object::Ptr JSON_FROM_FILE();
    void createDirectory(std::string filePath);

private:
    bool checkIP(std::string, std::string);
    bool checkMac(std::string);
    bool isLocalhost(const std::string &);
    bool isSameIP(const std::string &, const std::string &);

    // Add for logging database
    std::string reqClientAddress;

    std::chrono::steady_clock::time_point startStamp;
    Poco::Util::XMLConfiguration *xml_config;
    std::string ConfigFile;
    // logger
    Poco::AutoPtr<Poco::FormattingChannel> channel;
    void setLogPath();

    // fileserver function
    void uploadFile(std::weak_ptr<StreamSocket>, MemoryInputStream &, HTTPRequest &);
    void downloadFile(std::weak_ptr<StreamSocket>, MemoryInputStream &, HTTPRequest &);
    void deleteFile(std::weak_ptr<StreamSocket>, MemoryInputStream &, HTTPRequest &);
    void updateFile(std::weak_ptr<StreamSocket>, MemoryInputStream &, HTTPRequest &);
    void saveInfo(std::weak_ptr<StreamSocket>, MemoryInputStream &, HTTPRequest &);
    std::string template_dir;

private:
    const std::string YAMLTEMPL = R"MULTILINE(
        swagger: '2.0'
        info:
          version: v1
          title: ODF Template Center API
          description: ''
        host: '%s'
        paths:
          /lool/templaterepo/list:
            get:
              responses:
                '200':
                  description: Success
                  schema:
                    type: object
                    properties:
                      Category:
                        type: array
                        items:
                          $ref: '#/definitions/Category'
          /lool/templaterepo/sync:
            post:
              consumes:
                - application/json
              parameters:
                - $ref : '#/parameters/Sync'
              responses:
                '200':
                  description: Success
                '401':
                  description: Json data error
        schemes:
          - http
          - https
        definitions:
          Category:
            type: object
            required:
              - uptime
              - endpt
              - cid
              - hide
              - extname
              - docname
            properties:
              uptime:
                type: string
                format: date-time
              endpt:
                type: string
              cid:
                type: string
              hide:
                type: string
              extname:
                type: string
              docname:
                type: string

        parameters:
          Sync:
            in: body
            name: body
            description: ''
            required: true
            schema:
              type: object
              properties:
                Category:
                  type: array
                  items:
                    $ref: '#/definitions/Category'

        )MULTILINE";
};

#endif
