
#ifndef __MERGEODF_PARSER_H__
#define __MERGEODF_PARSER_H__
#include "config.h"
#include "mergeodf.h"
#include <string>

#include <Poco/DOM/DOMParser.h>
#include <Poco/DOM/Document.h>
#include <Poco/DOM/Element.h>
#include <Poco/DOM/AutoPtr.h>
#include <Poco/DOM/NodeList.h>
#include <Poco/DOM/DOMWriter.h>
#include <Poco/DOM/Text.h>
#include <Poco/JSON/Object.h>
#include <Poco/Path.h>
#include <Poco/File.h>
#include <Poco/TemporaryFile.h>
#include <Poco/URI.h>
#include <Poco/StringTokenizer.h>
#include <Poco/Base64Decoder.h>
#include <Poco/Dynamic/Var.h>
#include <Poco/Zip/Decompress.h>
#include <Poco/Zip/Compress.h>
#include <Poco/SAX/InputSource.h>

using Poco::XML::DOMParser;
using Poco::XML::DOMWriter;
using Poco::XML::Element;
using Poco::XML::InputSource;
using Poco::XML::NodeList;
using Poco::XML::Node;;
using Poco::XML::DOMWriter;
using Poco::XML::AutoPtr;
using Poco::XML::XMLReader;
using Poco::TemporaryFile;
using Poco::JSON::Object;
using Poco::URI;
using Poco::Path;
using Poco::File;
using Poco::Dynamic::Var;
using Poco::StringTokenizer;
using Poco::JSON::Array;
using Poco::Zip::Decompress;
using Poco::Zip::Compress;

class Parser
{
    public:
        enum class DocType
        {
            OTHER,
            TEXT,
            SPREADSHEET
        };

        const int tokenOpts = StringTokenizer::TOK_IGNORE_EMPTY |
                              StringTokenizer::TOK_TRIM;
        Parser(std::string);
        Parser(URI&);

        ~Parser();

        std::string getMimeType();
        void extract(std::string);

        std::string jsonVars();
        std::string jjsonVars();
        std::string yamlVars();

        std::vector<std::list<Element*>> scanVarPtr();
        std::string zipback();

        void updatePic2MetaXml();

        bool isValid();

        void setOutputFlags(bool, bool);
        std::string varKeyValue(std::string, std::string);
        void setSingleVar(Object::Ptr, std::list<Element*> &);
        void setGroupVar(Object::Ptr, std::list<Element*> &);
        std::string jsonvars;   // json 說明 - openapi
        std::string jjsonvars;  // json 範例
        std::string yamlvars;   // yaml
    private:
        DocType doctype;
        bool success;
        unsigned picserial;

        bool outAnotherJson;
        bool outYaml;

        std::map < std::string, Path > zipfilepaths;
        AutoPtr<Poco::XML::Document> docXML;
        std::list<Element*> groupAnchorsSc;

        std::string extra2;
        std::string contentXml;
        std::string contentXmlFileName;
        std::string metaFileName;

        void detectDocType();
        bool isText();
        bool isSpreadSheet();

        std::string replaceMetaMimeType(std::string);
        void updateMetaInfo();

        std::string parseEnumValue(std::string, std::string, std::string);
        std::string parseJsonVar(std::string, std::string, bool, bool);

        const std::string PARAMTEMPL = R"MULTILINE(
                    "%s": {
                        "type": "%s"%s
                    })MULTILINE";
        const std::string PARAMGROUPTEMPL = R"MULTILINE(
                      "%s": {
                        "type": "array",
                        "xml": {
                            "name": "%s",
                            "wrapped": true
                        },
                        "items": {
                          "type": "object",
                          "properties": {
                            %s
                          }
                        }
                      },)MULTILINE";
        const std::string YAMLPARAMTEMPL = R"MULTILINE(
                                    "%s":
                                        "type": "%s"
                                        %s
            )MULTILINE";
        const std::string YAMLPARAMGROUPTEMPL = R"MULTILINE(
                                    "%s":
                                        "type": "array"
                                        "xml":
                                            "name": "%s"
                                            "wrapped": true
                                        "items":
                                            "type": "object"
                                            "properties":
                                                %s
            )MULTILINE";
};

#endif
