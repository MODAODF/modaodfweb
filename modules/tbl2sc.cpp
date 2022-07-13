#include "tbl2sc.h"
#include "tbl2sc_logging_db.h"
#include "LOOLWSD.hpp"
#include "oxoolmodule.h"
#include <stdlib.h>
#include <iostream>
#include <assert.h>
#include "JsonUtil.hpp"

#define LOK_USE_UNSTABLE_API
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <LibreOfficeKit/LibreOfficeKit.hxx>

#include <Poco/RegularExpression.h>
#include <Poco/Net/HTMLForm.h>
#include <Poco/Delegate.h>
#include <Poco/Zip/Compress.h>
#include <Poco/Zip/Decompress.h>
#include <Poco/Glob.h>
#include <Poco/TemporaryFile.h>
#include <Poco/Format.h>
#include <Poco/StringTokenizer.h>
#include <Poco/StreamCopier.h>
#include <Poco/DOM/DOMParser.h>
#include <Poco/DOM/Document.h>
#include <Poco/SAX/InputSource.h>
#include <Poco/DOM/DOMWriter.h>
#include <Poco/DOM/NodeList.h>
#include <Poco/DOM/Text.h>
#include <Poco/Util/Application.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/DOM/AutoPtr.h>
#include <Poco/FileStream.h>
#include <Poco/Exception.h>
#include <Poco/FileChannel.h>
#include <Poco/PatternFormatter.h>

#include <Poco/Base64Encoder.h>
#include <Poco/Base64Decoder.h>
using Poco::Net::HTMLForm;
using Poco::Net::MessageHeader;
using Poco::Net::HTTPResponse;
using Poco::RegularExpression;
using Poco::Zip::Compress;
using Poco::Zip::Decompress;
using Poco::Path;
using Poco::File;
using Poco::TemporaryFile;
using Poco::StreamCopier;
using Poco::StringTokenizer;
using Poco::XML::DOMParser;
using Poco::XML::DOMWriter;
using Poco::XML::Element;
using Poco::XML::InputSource;
using Poco::XML::NodeList;
using Poco::XML::Node;;
using Poco::Util::Application;
using Poco::XML::XMLReader;
using Poco::FileOutputStream;
using Poco::IOException;
using Poco::Util::Application;
using Poco::FileChannel;
using Poco::PatternFormatter;


std::string addSlashes(const std::string &source)
{
    std::string out;
    for (const char c: source)
    {
        switch (c)
        {
            case '\\':  out += "\\\\";  break;
            default:    out += c;       break;
        }
    }
    return out;
}

static Poco::Logger& logger()
{
    return Application::instance().logger();
}

Tbl2SC::Tbl2SC()
{
    ConfigFile = LOOLWSD_MODULE_CONFIG_DIR "/tbl2sc.xml";
    startStamp = std::chrono::steady_clock::now();
    xml_config = new Poco::Util::XMLConfiguration(ConfigFile);

    setProgPath(LOOLWSD::LoTemplate);
    setLogPath();
    Application::instance().logger().setChannel(channel);
}

Tbl2SC::~Tbl2SC()
{}

/// init. logger
/// 設定 log 檔路徑後直接 init.
void Tbl2SC::setLogPath()
{
    Poco::AutoPtr<FileChannel> fileChannel(new FileChannel);
    std::string logFile = xml_config->getString("logging.log_file", "/tmp/tbl2sc.log");
    fileChannel->setProperty("path", logFile);
    fileChannel->setProperty("archive", "timestamp");
    fileChannel->setProperty("rotation", "weekly");
    fileChannel->setProperty("compress", "true");
    Poco::AutoPtr<PatternFormatter> patternFormatter(new PatternFormatter());
    patternFormatter->setProperty("pattern","%Y/%m/%d %L%H:%M:%S: %t");
    channel = new Poco::FormattingChannel(patternFormatter, fileChannel);
}

/// check if uri valid this ap.
bool Tbl2SC::isTbl2SCUri(std::string uri)
{
    StringTokenizer tokens(uri, "/?");
    return (tokens.count() == 3 && tokens[2] == "tbl2sc");
}

/// validate for form args
bool Tbl2SC::validate(const HTMLForm &form,
        std::weak_ptr<StreamSocket> _socket)
{
    HTTPResponse response;
    response.set("Access-Control-Allow-Origin", "*");
    response.set("Access-Control-Allow-Methods", "POST, OPTIONS");
    response.set("Access-Control-Allow-Headers",
            "Origin, X-Requested-With, Content-Type, Accept");

    auto socket = _socket.lock();

    if (!form.has("format"))
    {
        response.setStatusAndReason(HTTPResponse::HTTP_BAD_REQUEST,
                "parameter format must assign");
        response.setContentLength(0);
        socket->send(response);
        socket->shutdown();
        return false;
    }
    const std::string format = form.get("format");
    if (format != "ods" && format != "pdf")
    {
        response.setStatusAndReason(HTTPResponse::HTTP_BAD_REQUEST,
                "wrong parameter format");
        response.setContentLength(0);
        socket->send(response);
        socket->shutdown();
        return false;
    }
    if (!form.has("title"))
    {
        response.setStatusAndReason(HTTPResponse::HTTP_BAD_REQUEST,
                "parameter title must assign");
        response.setContentLength(0);
        socket->send(response);
        socket->shutdown();
        return false;
    }
    if (!form.has("content"))
    {
        response.setStatusAndReason(HTTPResponse::HTTP_BAD_REQUEST,
                "parameter content must assign");
        response.setContentLength(0);
        socket->send(response);
        socket->shutdown();
        return false;
    }
    const std::string title = form.get("title");
    std::string content = form.get("content");

    Poco::RegularExpression re(".*<table( [^<]*)*>.*</table>.*",
            Poco::RegularExpression::RE_DOTALL);
    if (!re.match(content))
    {
        response.setStatusAndReason(HTTPResponse::HTTP_BAD_REQUEST,
                "no <table>");
        response.setContentLength(0);
        socket->send(response);
        socket->shutdown();
        return false;
    }
    return true;
}

/// callback for lok
void Tbl2SC::ViewCallback(const int type,
        const char* p,
        void* data)
{
    std::cout << "CallBack Type: " << type << std::endl;
    std::cout << "payload: " << p << std::endl;
    Tbl2SC* self = reinterpret_cast<Tbl2SC*>(data);
    if (type == LOK_CALLBACK_UNO_COMMAND_RESULT)
    {
        self->isUnoCompleted = true;
    }
}

/// convert using soffice.so
std::string Tbl2SC::outputODF(const std::string odffile,
        const std::string format)
{
    if (odffile.empty())
    {
        return "";
    }

    std::string outfile;
    lok::Office *llo = NULL;
    Path tempPath = Path::forDirectory(
            Poco::TemporaryFile::tempName() + "/");
    auto userprofile = File(tempPath);
    userprofile.createDirectories();
    try
    {
        ::setenv("UNODISABLELIBRARY",
                "abp avmediagst avmediavlc cmdmail losessioninstall OGLTrans PresenterScreen "
                "syssh ucpftp1 ucpgio1 ucphier1 ucpimage updatecheckui updatefeed updchk"
                // Database
                "dbaxml dbmm dbp dbu deployment firebird_sdbc mork "
                "mysql mysqlc odbc postgresql-sdbc postgresql-sdbc-impl sdbc2 sdbt"
                // Java
                "javaloader javavm jdbc rpt rptui rptxml ",
                0 /* no overwrite */);
        unsetenv("DISPLAY");
        std::string userprofile_uri = "file://" + tempPath.toString();
        llo = lok::lok_cpp_init(loPath.c_str(), userprofile_uri.c_str());
        if (!llo)
        {
            std::cout << ": Failed to initialise LibreOfficeKit" << std::endl;
            return "Failed to initialise LibreOfficeKit";
        }
    }
    catch (const std::exception & e)
    {
        delete llo;
        std::cout << ": LibreOfficeKit threw exception (" << e.what() << ")" << std::endl;
        return e.what();
    }

    char *options = 0;
    lok::Document * lodoc = llo->documentLoad(odffile.c_str(), options);
    if (!lodoc)
    {
        const char * errmsg = llo->getError();
        std::cerr << ": LibreOfficeKit failed to load document (" << errmsg << ")" << std::endl;
        return errmsg;
    }
    lodoc->registerCallback(ViewCallback, this);

    //Run postUnocommand
    lodoc->postUnoCommand(".uno:SelectAll", nullptr, true);
    std::string json = "{\"aExtraWidth\":{\"type\":\"unsigned short\",\"value\":200}}";
    lodoc->postUnoCommand(".uno:SetOptimalColumnWidthDirect", json.c_str(), true);

    outfile = odffile + ".odf";
    //std::cout << outfile << std::endl;
    if (!lodoc->saveAs(outfile.c_str(), format.c_str(), options))
    {
        const char * errmsg = llo->getError();
        std::cerr << ": LibreOfficeKit failed to export (" << errmsg << ")" << std::endl;

        delete lodoc;
        return errmsg;
    }

    return outfile;
}

// XML 操作

/// 將 ODF 檔案解開
std::tuple<std::string, std::string> extract(std::string templfile)
{
    /*
     * templfile : 要解開的 ODF 檔案
     */
    std::string extra2 = TemporaryFile::tempName();

    std::ifstream inp(templfile, std::ios::binary);
    assert (inp.good());
    Decompress dec(inp, extra2);
    dec.decompressAllFiles();
    assert (!dec.mapping().empty());

    std::map < std::string, Path >  zipfilepaths = dec.mapping();
    std::string contentXmlFileName, metaFileName;
    for (auto it = zipfilepaths.begin(); it != zipfilepaths.end(); ++it)
    {
        const auto fileName = it->second.toString();
        if (fileName == "content.xml")
            contentXmlFileName = extra2 + "/" + fileName;

        if (fileName == "META-INF/manifest.xml")
            metaFileName = extra2 + "/" + fileName;
    }

    return std::make_tuple(contentXmlFileName, extra2);
}

void saveXmlBack(Poco::XML::AutoPtr<Poco::XML::Document> docXML,
        std::string xmlfile)
{
    /*
     * docXML : Poco 讀出來的 XML 檔案
     * xmlfile : XML 的檔案名稱
     */
    std::ostringstream ostrXML;
    DOMWriter writer;
    writer.writeNode(ostrXML, docXML);
    const auto xml = ostrXML.str();

    Poco::File f(xmlfile);
    f.setSize(0);  // truncate

    Poco::FileOutputStream fos(xmlfile, std::ios::binary);
    fos << xml;
    fos.close();
}

std::string zipback(std::string extra2, Poco::XML::AutoPtr<Poco::XML::Document> docXML,
        std::string contentXmlFileName)
{
    /*
     * extra2 : 要壓縮的資料夾根目錄路徑
     * docXML : Poco 讀出來的 XML 檔案
     * contentXmlFileName : 一般來說就是 ODF 檔案解出來的 content.xml 的路徑
     *
     */
    saveXmlBack(docXML, contentXmlFileName);

    // zip
    const auto zip2 = extra2 + ".ods";
    std::cout << "zip2: " << zip2 << std::endl;

    std::ofstream out(zip2, std::ios::binary);
    Compress c(out, true);

    c.addRecursive(extra2);
    c.close();
    return zip2;
}


std::string customizeSC(std::string filepath, std::string font, std::string oddRowColor)
{
    // Check optional value of the font and oddRowColor
    if(font == "TimesNewRoman")
        font = "Times New Roman";
    if(font != "Times New Roman" && font!= "微軟正黑體" && font!="標楷體")
        font = "微軟正黑體";
    if(oddRowColor == "")
        oddRowColor = "#9d9d9d";

    // load the xml file to program
    std::tuple<std::string, std::string> unzipPath = extract(filepath);
    auto contentXmlFilePath = std::get<0>(unzipPath);
    auto extraDirectory = std::get<1>(unzipPath);
    InputSource sourceFile(contentXmlFilePath);
    DOMParser parser;
    parser.setFeature(XMLReader::FEATURE_NAMESPACES, false);
    parser.setFeature(XMLReader::FEATURE_NAMESPACE_PREFIXES, true);
    Poco::XML::AutoPtr<Poco::XML::Document> docXML;
    docXML = parser.parse(&sourceFile);

    auto style_deep = docXML->createElement("style:style");

    //Set style Node
    style_deep->setAttribute("style:name", "looldark");
    style_deep->setAttribute("style:family", "table-cell");
    style_deep->setAttribute("style:parent-style-name", "Default");

    //Set style:table-cell-properties
    auto style_cell_property = docXML->createElement("style:table-cell-properties");
    style_cell_property->setAttribute("fo:background-color", oddRowColor); // Assign rowColor here
    style_cell_property->setAttribute("fo:border", "0.06pt solid #000000");

    //Set style text-properties
    auto style_text_prop = docXML->createElement("style:text-properties");
    style_text_prop->setAttribute("style:font-name",font); // Assign font here
    style_text_prop->setAttribute("style:use-window-font-color","true");
    style_text_prop->setAttribute("style:text-outline","false");
    style_text_prop->setAttribute("style:text-line-through-style","none");
    style_text_prop->setAttribute("style:text-line-through-type","none");
    style_text_prop->setAttribute("fo:font-size","10pt");
    style_text_prop->setAttribute("fo:language","en");
    style_text_prop->setAttribute("fo:country","US");
    style_text_prop->setAttribute("fo:font-style","normal");
    style_text_prop->setAttribute("fo:text-shadow","none");
    style_text_prop->setAttribute("style:text-underline-style","none");
    style_text_prop->setAttribute("fo:font-weight","normal");
    style_text_prop->setAttribute("style:text-underline-mode","continuous");
    style_text_prop->setAttribute("style:text-overline-mode","continuous");
    style_text_prop->setAttribute("style:text-line-through-mode","continuous");
    style_text_prop->setAttribute("style:font-size-asian","10pt");
    style_text_prop->setAttribute("style:language-asian","zh");
    style_text_prop->setAttribute("style:country-asian","TW");
    style_text_prop->setAttribute("style:font-style-asian","normal");
    style_text_prop->setAttribute("style:font-weight-asian","normal");
    style_text_prop->setAttribute("style:font-size-complex","10pt");
    style_text_prop->setAttribute("style:language-complex","hi");
    style_text_prop->setAttribute("style:country-complex","IN");
    style_text_prop->setAttribute("style:font-style-complex","normal");
    style_text_prop->setAttribute("style:font-weight-complex","normal");
    style_text_prop->setAttribute("style:text-emphasize","none");
    style_text_prop->setAttribute("style:font-relief","none");
    style_text_prop->setAttribute("style:text-overline-style","none");
    style_text_prop->setAttribute("style:text-overline-color","font-color");

    //Add prop to style
    style_deep->appendChild(style_text_prop);
    style_deep->appendChild(style_cell_property);

    //Clone a new style to white bg
    auto style_light = docXML->createElement("style:style");
    style_light->setAttribute("style:name", "loollight");
    style_light->setAttribute("style:family", "table-cell");
    style_light->setAttribute("style:parent-style-name", "Default");

    auto light_style_cell = static_cast<Element*> (style_cell_property->cloneNode(true));
    auto light_style_text = style_text_prop->cloneNode(true);
    light_style_cell->setAttribute("fo:background-color", "#ffffff");
    style_light->appendChild(light_style_text);
    style_light->appendChild(light_style_cell);

    //Add new style to docXML
    auto styleNode = static_cast<Element *> (docXML->getElementsByTagName("office:automatic-styles")->item(0));
    styleNode->appendChild(style_deep);
    styleNode->appendChild(style_light);

    //Add style to cell
    auto rowList = docXML->getElementsByTagName("table:table-row");
    int rowLen = rowList->length();
    std::string styleName = "looldark";
    for ( int i = 0; i < rowLen; i++)
    {
        if(i%2 != 0)
            styleName = "loollight";
        else
            styleName = "looldark";
        auto elm = static_cast<Element *> (rowList->item(i));
        auto childs = elm->getElementsByTagName("table:table-cell");
        for(unsigned int j=0; j < childs->length();j++){
            auto child = static_cast<Element*>(childs->item(j));
            child->setAttribute("table:style-name", styleName);
        }
    }
    std::string outputPath = "";
    try{
        outputPath = zipback(extraDirectory, docXML, contentXmlFilePath);
    }
    catch(Poco::IOException &e){
        std::cout<<e.displayText()<<std::endl;
        return outputPath;
    }
    return outputPath;
}
/// 轉檔:
/// <table>...</table>  轉成  ods or pdf
void Tbl2SC::doTbl2SC(const Poco::Net::HTTPRequest& request,
        Poco::MemoryInputStream& message,
        std::weak_ptr<StreamSocket> _socket)
{
    HTMLForm form(request, message);
    //validate the post parameter
    if (!validate(form, _socket))
        return;

    const std::string format = form.get("format");
    const std::string title = form.get("title");
    const std::string content = form.get("content");
    const std::string font = form.get("font", "");
    const std::string oddRowColor = form.get("oddRowColor", "");


    const std::string templ = R"MULTILINE(
        <!doctype html>
        <html lang="zh-tw">
        <head>
            <title>%s</title>
            <meta charset="UTF-8">
            <meta http-equiv="X-UA-Compatible" content="IE=edge">
        </head>
        <body>
        %s
        </body>
        </html>
        )MULTILINE";


    auto buf = Poco::format(templ, title, content);

    auto sourcefile = TemporaryFile::tempName() + ".xls";

    Poco::File f(sourcefile);
    f.createFile();

    Poco::FileOutputStream fos(sourcefile, std::ios::binary);
    fos << buf;
    fos.close();
    std::cout << "sourcefile: " << sourcefile << std::endl;


    auto socket = _socket.lock();
    //outputfile is filepath located in /tmp/ generated by poco tempfile
    auto outputfile = outputODF(sourcefile, format);
	std::cout << "outputfile: " << outputfile <<"\n";
    Poco::File checkFile_step1(outputfile);
    if (outputfile.empty())
    {
        quickHttpRes(_socket, HTTPResponse::HTTP_INTERNAL_SERVER_ERROR, "轉檔失敗");
        log(false, title, "轉檔失敗");
        logger().notice("轉檔失敗, req from: " + reqClientAddress);
        return;
    }
    else if(!outputfile.empty() && !checkFile_step1.exists())
    {
        quickHttpRes(_socket, HTTPResponse::HTTP_INTERNAL_SERVER_ERROR, "轉檔失敗");
        log(false, title, "轉檔失敗");
        logger().notice("轉檔失敗, req from: " + reqClientAddress);
        return;
    }


    // customize the output file
    outputfile = customizeSC(outputfile, font, oddRowColor);
    Poco::File checkFile_step2(outputfile);
    if (outputfile.empty())
    {
        quickHttpRes(_socket, HTTPResponse::HTTP_INTERNAL_SERVER_ERROR, "轉檔失敗(2)");
        log(false, title, "轉檔失敗(2)");
        logger().notice("轉檔失敗(2), req from: " + reqClientAddress);
        return;
    }
    else if(!outputfile.empty() && !checkFile_step2.exists())
    {
        quickHttpRes(_socket, HTTPResponse::HTTP_INTERNAL_SERVER_ERROR, "轉檔失敗(2)");
        log(false, title, "轉檔失敗(2)");
        logger().notice("轉檔失敗(2), req from: " + reqClientAddress);
        return;

    }

    HTTPResponse response;
    response.set("Access-Control-Allow-Origin", "*");
    response.set("Access-Control-Allow-Methods",
            "POST, OPTIONS");
    response.set("Access-Control-Allow-Headers",
            "Origin, X-Requested-With, Content-Type, Accept");

    HttpHelper::sendFileAndShutdown(socket, outputfile,
            getMimeType(outputfile), &response);

    logger().notice("Successfully transform, req from: " + reqClientAddress);
    log(true, title, "OK");
}

void Tbl2SC::log(bool success,
                std::string title,
                std::string e_msg)
{
    const auto timeSinceStartMs =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - startStamp).count();
    std::string timeSince = std::to_string(timeSinceStartMs * 0.001);

    // Store in SQLite
    LogDB log_db;
    Poco::Timestamp tstmp;
    std::string datetime = Poco::DateTimeFormatter::format(tstmp, "%Y-%n-%e %H:%M:%S");
    log_db.addRecord(success, reqClientAddress, title,
            timeSince, datetime, e_msg);
}

void Tbl2SC::handleRequest(std::weak_ptr<StreamSocket> _socket,
        MemoryInputStream& message,
        HTTPRequest& request,
        SocketDisposition& disposition)
{
    StringTokenizer clientaddress_tokens(_socket.lock()->clientAddress(), ":");
    reqClientAddress = clientaddress_tokens[clientaddress_tokens.count()-1];
    Process::PID pid = fork();
    if (pid < 0)
    {
        quickHttpRes(_socket, HTTPResponse::HTTP_INTERNAL_SERVER_ERROR, "Server process Error");
    }
    else if (pid == 0)
    {
        try{
            if(false)
                disposition.setClosed();
            Poco::URI requestUri(request.getURI());
            std::vector<std::string> reqPathSegs;
            requestUri.getPathSegments(reqPathSegs);
            std::string method = request.getMethod();
            if (request.getContentType() == "application/json")
            {
                std::string line, data;
                std::istream &iss(message);
                while (!iss.eof())
                {
                    std::getline(iss, line);
                    data += line;
                }
            }
            if (request.getMethod() == HTTPRequest::HTTP_GET && reqPathSegs.size()==2)
            {
                quickHttpRes(_socket, HTTPResponse::HTTP_OK, "OK");
            }
            else if (
                    request.getMethod() != HTTPRequest::HTTP_GET &&
                    isTbl2SCUri(request.getURI()))
            {  // /lool/tbl2sc
                doTbl2SC(request, message, _socket);
            }
            else
            {
                quickHttpRes(_socket, HTTPResponse::HTTP_NOT_FOUND, "無此API");
            }
        }
        catch(const std::exception &e)
        {
            std::cout << e.what() << std::endl;
            logger().notice("[Exception]" + std::string(e.what()));
            quickHttpRes(_socket, HTTPResponse::HTTP_INTERNAL_SERVER_ERROR, "Please contact the adminmanager");
        }

        try
        {
            auto socket = _socket.lock();
            socket->shutdown();
        }
        catch(const std::exception &e)
        {
            std::cout<< "Force shutdown socket in module\n";
        }
        exit_application = true;
    }
    else
    {
        std::cout << "call from parent\n";
    }
}

std::string Tbl2SC::handleAdmin(std::string command)
{
    /*
     *command format: module <modulename> <action> <data in this format: x,y,z ....>
     */
    auto tokenOpts = StringTokenizer::TOK_IGNORE_EMPTY | StringTokenizer::TOK_TRIM;
    StringTokenizer tokens(command, " ", tokenOpts);
    std::string result = "Module Loss return";
    std::string action = tokens[2];
    std::string dataString = tokens.count() >= 4 ? tokens[3] : "";

    if(action == "getLog")
    {
        std::cout << "getLog\n";
        result = "getLog OK";
    }
    else if (action == "getModuleVersion")
    {
        result = action + " " + std::string(PACKAGE_VERSION);
    }
    else if(action == "get_log" && tokens.count() > 3)
    {
        LogDB *log_db = new LogDB();
        std::string rec_id = tokens[3];
        result = action + " " + log_db->getRecord(rec_id);
    }
    // 以 json 字串傳回 oxoolwsd.xml 項目參考 oxool Admin.cpp
    else if (action == "getConfig" && tokens.count() > 4)
    {
        std::ostringstream oss;
        oss << "settings {\n";
        for (size_t i=3; i < tokens.count() ; i ++)
        {
            std::string key = tokens[i];

            if (i > 3) oss << ",\n";

            oss << "\"" << key << "\": ";
            // 下列三種 key 是陣列形式

            if (xml_config->has(key))
            {
                std::string p_value = addSlashes(xml_config->getString(key, "")); // 讀取 value, 沒有的話預設為空字串
                std::string p_type = xml_config->getString(key + "[@type]", ""); // 讀取 type, 沒有的話預設為空字串
                if (p_type == "int" || p_type == "uint" || p_type == "bool" ||
                    p_value == "true" || p_value=="false")
                    oss << p_value;
                else
                    oss << "\"" << p_value << "\"";
            }
            else
            {
                oss << "null";
            }
        }
        oss << "\n}\n";
        result = oss.str();
    }
    else if (action == "setConfig" && tokens.count() > 1)
    {
        Poco::JSON::Object::Ptr object;
        if (JsonUtil::parseJSON(command, object))
        {
            for (Poco::JSON::Object::ConstIterator it = object->begin(); it != object->end(); ++it)
            {
                // it->first : key, it->second.toString() : value
                xml_config->setString(it->first, it->second.toString());
            }
            xml_config->save(ConfigFile);
            result = "setConfigOk";
        }
        else
        {
            result = "setConfigNothing";
        }
    }
    else
        result = "No such command for module " + tokens[1];

    //std::cout << tokens[2] << " : " << result << std::endl;
    return result;
}

extern "C" Tbl2SC *create_object()
{
    return new Tbl2SC;
}

extern "C" void destroy_object(Tbl2SC *tbl2sc)
{
    delete tbl2sc;
}
