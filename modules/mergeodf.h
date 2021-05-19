#ifndef __MERGEODF_H__
#define __MERGEODF_H__
#include "config.h"
#include "Socket.hpp"
#include "oxoolmodule.h"
#include "mergeodf_access_db.h"

#include <Poco/Tuple.h>
#include <Poco/FileStream.h>
#include <Poco/Net/HTMLForm.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/MemoryStream.h>
#include <Poco/URI.h>
#include <Poco/UUID.h>
#include <Poco/UUIDGenerator.h>
#include <Poco/Process.h>
#include <Poco/Util/LayeredConfiguration.h>
#include <Poco/Util/XMLConfiguration.h>
#include <Poco/JSON/Object.h>
#include <Poco/Glob.h>
#include <Poco/AutoPtr.h>

using Poco::Path;
using Poco::URI;
using Poco::Net::NameValueCollection;
using Poco::Net::HTTPRequest;
using Poco::Net::HTMLForm;
using Poco::MemoryInputStream;
using Poco::Process;
using Poco::JSON::Object;
typedef Poco::Tuple<std::string, std::string> VarData;

const std::string resturl = "/lool/mergeodf/";

// @TODO: 怪？設定這個變數值以後，lool stop 就會 double free error!
//const std::string Parser::TAG_VARDATA_SC = "office:target-frame-name";

class MergeODF : public oxoolmodule
{
    public:
        // oxoolmodule interface
        void handleRequest(std::weak_ptr<StreamSocket>, MemoryInputStream&, HTTPRequest&, SocketDisposition&);
        std::string getHTMLFile(std::string);
        // fileserver function
        void uploadFile(std::weak_ptr<StreamSocket>, MemoryInputStream&, HTTPRequest&);
        void downloadFile(std::weak_ptr<StreamSocket>, MemoryInputStream&, HTTPRequest&);
        void deleteFile(std::weak_ptr<StreamSocket>, MemoryInputStream&, HTTPRequest&);
        void updateFile(std::weak_ptr<StreamSocket>, MemoryInputStream&, HTTPRequest&);
        MergeODF();
        virtual ~MergeODF();
        std::string handleAdmin(std::string);

        enum class MergeStatus
        {
            OK,
            PARAMETER_REQUIRE,  /// @TODO: unused?
            TEMPLATE_NOT_FOUND,
            JSON_PARSE_ERROR,
            HTMLFORM_PARSE_ERROR
        };

        virtual void setProgPath(std::string path)
        {
            loPath = path + "/program";
        }

        std::string ConfigFile;
        Poco::Util::XMLConfiguration *xml_config;
        virtual void setLogPath();
        virtual void initSQLDB();

        virtual std::string isMergeToUri(std::string,
                bool forHelp=false,
                bool anotherJson=false,
                bool yaml=false);
        virtual std::string isMergeToHelpUri(std::string,
                bool anotherJson=false,
                bool yaml=false);
        virtual std::string isMergeToQueryAccessTime(std::string);
        virtual std::string makeApiJson(std::string,
                bool anotherJson=false,
                bool yaml=false,
                bool showHead=true);
        virtual void handleMergeTo(std::weak_ptr<StreamSocket>,
                const Poco::Net::HTTPRequest&,
                Poco::MemoryInputStream&);
        virtual int getApiCallTimes(std::string);
        virtual void responseAccessTime(std::weak_ptr<StreamSocket>, std::string);

    private:
        AccessDB *access_db;
        // Add for logging database
        std::string reqClientAddress;
        void log(bool success, std::string endpt, std::string e_msg, bool toPDF);
        std::chrono::steady_clock::time_point startStamp;
        std::string loPath;  // soffice program path
        std::string template_dir;
        Poco::AutoPtr<Poco::Channel> channel;

        std::string mimetype;
        MergeStatus mergeStatus;

        std::string getDocExt();
        MergeStatus getMergeStatus();
        std::string doMergeTo(const HTTPRequest&, MemoryInputStream&);

        std::list<std::string> getTemplLists(bool);
        std::string getContentFromTemplFile(std::string);
        std::list<std::string> getVarsFromTempl(std::string, bool);
        std::list<std::string> getVarsFromUri(std::string);
        std::string keyword2Lower(std::string, std::string);
        bool parseJson(HTMLForm &);
        Object::Ptr parseArray2Form(HTMLForm &);
        void handleAPIHelp(std::weak_ptr<StreamSocket>,
                bool showMerge=true,
                std::string mergeEndPoint="",
                bool anotherJson=false,
                bool yaml=false);

        std::string outputODF(std::string);

        const std::string TEMPLH = R"MULTILINE(
            {
                "swagger": "2.0",
                "info": {
                    "version": "v1",
                    "title": "ODF 報表 API",
                    "description": ""
                },
                "host": "%s",
                "paths": {
                    %s
                },
                "schemes": [
                    "http",
                    "https"
                ],
                "parameters": {
                    "outputPDF": {
                        "in": "query",
                        "name": "outputPDF",
                        "required": false,
                        "type": "boolean",
                        "allowEmptyValue": true,
                        "description": "轉輸出成 PDF 格式"
                    }
                }
            }
        )MULTILINE";


        const std::string APITEMPL = R"MULTILINE(
            "/lool/mergeodf/%s/accessTime": {
              "get": {
                "consumes": [
                  "multipart/form-data",
                  "application/json"
                ],
                "responses": {
                  "200": {
                    "description": "傳送成功",
                    "schema": {
                      "type": "object",
                      "properties": {
                        "call_time": {
                          "type": "integer",
                          "description": "呼叫次數."
                        }
                      }
                    }
                  },
                  "503": {
                    "description": "server_name 無指定"
                  },
                }
              }
            },
            "/lool/mergeodf/%s": {
                "post": {
                    "consumes": [
                        "multipart/form-data",
                        "application/json"
                    ],
                "parameters": [
                  {
                    "$ref": "#/parameters/outputPDF"
                  },
                  {
                    "in": "body",
                    "name": "body",
                    "description": "",
                    "required": true,
                    "schema": {
                        "type": "object",
                        "properties": {%s}
                        }
                  }
                ],
                "responses": {
                  "200": {
                    "description": "傳送成功"
                  },
                  "400": {
                    "description": "Json 格式錯誤 / form 格式錯誤"
                  },
                  "404": {
                    "description": "無此 API"
                  },
                  "500": {
                    "description": "轉換失敗 / 輸出 PDF 錯誤"
                  },
                  "503": {
                    "description": "模組尚未授權"
                  }

                }
              }
            }
        )MULTILINE";

        const std::string YAMLTEMPLH = R"MULTILINE(
            swagger: '2.0'
            info:
              version: v1
              title: ODF 報表 API
              description: ''
            host: %s
            paths:%s
            schemes: ["http", "https"]
            parameters:
              outputPDF:
                in: query
                name: outputPDF
                required: false
                type: boolean
                allowEmptyValue : true
                description: 轉輸出成 PDF 格式
        )MULTILINE";
        const std::string YAMLTEMPL = R"MULTILINE(
                /lool/mergeodf/%s/accessTime:
                    get:
                      consumes:
                        - application/json
                      responses:
                        '200':
                            description: 傳送成功
                            schema:
                                type: object
                                properties:
                                    call_time:
                                        type: integer
                                        description: 呼叫次數
                        '503':
                            description: "server_name 無指定"
                /lool/mergeodf/%s:
                    post:
                        consumes:
                            - multipart/form-data
                            - application/json
                        parameters:
                            - $ref: '#/parameters/outputPDF'
                            - in: body
                              name: body
                              description: ''
                              required: false
                              schema:
                                  type: object
                                  properties:
                                    %s
                        responses:
                            '200':
                                description: '傳送成功'
                            '400':
                                description: 'Json 格式錯誤 / form 格式錯誤'
                            '404':
                                description: '無此 API'
                            '500':
                                description: '轉換失敗 / 輸出 PDF 錯誤'
                            '503':
                                description: '模組尚未授權'
        )MULTILINE";
};

/*
 * 模組共用
 */

/// 列目錄內的樣板檔
std::list<std::string> templLists(bool isBasename);

#endif // MERGEODF_H
