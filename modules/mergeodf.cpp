#include "mergeodf.h"
#include "LOOLWSD.hpp"
#include "JsonUtil.hpp"
#include "mergeodf_file_db.h"
#include "mergeodf_parser.h"
#include "mergeodf_logging_db.h"
#include "oxoolmodule.h"

#include <sys/wait.h>

#define LOK_USE_UNSTABLE_API
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <LibreOfficeKit/LibreOfficeKit.hxx>

#include <Poco/RegularExpression.h>
#include <Poco/Net/HTMLForm.h>
#include <Poco/Net/PartHandler.h>
#include <Poco/TemporaryFile.h>
#include <Poco/Format.h>
#include <Poco/StringTokenizer.h>
#include <Poco/StreamCopier.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Util/Application.h>
#include <Poco/FileChannel.h>
#include <Poco/FormattingChannel.h>
#include <Poco/PatternFormatter.h>

#include <Poco/Base64Encoder.h>
#include <Poco/Base64Decoder.h>

using Poco::Net::HTMLForm;
using Poco::Net::MessageHeader;
using Poco::Net::PartHandler;
using Poco::Net::HTTPResponse;
using Poco::RegularExpression;
using Poco::Path;
using Poco::File;
using Poco::TemporaryFile;
using Poco::StreamCopier;
using Poco::StringTokenizer;
using Poco::DynamicStruct;
using Poco::JSON::Array;
using Poco::Util::Application;
using Poco::FileChannel;
using Poco::PatternFormatter;


static Poco::Logger& logger()
{
    return Application::instance().logger();
}

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


/// 列目錄內的樣板檔
std::list<std::string> templLists(bool isBasename)
{
    std::set<std::string> files;
    std::list<std::string> rets;
#if ENABLE_DEBUG
    std::string template_dir = std::string(DEV_DIR) + "/runTimeData/mergeodf/";
#else
    auto mergeodfconf = new Poco::Util::XMLConfiguration("/etc/ndcodfweb/conf.d/mergeodf/mergeodf.xml");
    std::string template_dir = mergeodfconf->getString("template.dir_path", "");
#endif
    Poco::Glob::glob(template_dir + "*.ot[ts]", files);

    for (auto it = files.begin() ; it != files.end(); ++ it)
    {
        auto afile = *it;
        auto basename = Poco::Path(afile).getBaseName();
        if (isBasename)
            rets.push_back(basename);
        else
            rets.push_back(Poco::Path(afile).toString());
    }
    return rets;
}

/// Handles the filename part of the convert-to POST request payload.
class MergeODFPartHandler2 : public PartHandler
{
    public:
        NameValueCollection vars;  /// post filenames
    private:
        std::string& _filename;  /// current post filename

    public:
        MergeODFPartHandler2(std::string& filename)
            : _filename(filename)
        {
        }

        virtual void handlePart(const MessageHeader& header,
                std::istream& stream) override
        {
            // Extract filename and put it to a temporary directory.
            std::string disp;
            NameValueCollection params;
            if (header.has("Content-Disposition"))
            {
                std::string cd = header.get("Content-Disposition");
                MessageHeader::splitParameters(cd, disp, params);
            }

            if (!params.has("filename"))
                return;
            if (params.get("filename").empty())
                return;

            auto tempPath = Path::forDirectory(
                    TemporaryFile::tempName() + "/");
            File(tempPath).createDirectories();
            // Prevent user inputting anything funny here.
            // A "filename" should always be a filename, not a path
            const Path filenameParam(params.get("filename"));
            tempPath.setFileName(filenameParam.getFileName());
            _filename = tempPath.toString();

            // Copy the stream to _filename.
            std::ofstream fileStream;
            fileStream.open(_filename);
            StreamCopier::copyStream(stream, fileStream);
            fileStream.close();

            vars.add(params.get("name"), _filename);
            fprintf(stderr, "handle part, %s\n", _filename.c_str());
        }
};

class ManageFilePartHandler : public PartHandler
{
    public:
        NameValueCollection vars;  /// post filenames
    private:
        std::string _filename;  /// current post filename

    public:
        ManageFilePartHandler(std::string filename)
        :_filename(filename)
        {
        }

        virtual void handlePart(const MessageHeader& header,
                std::istream& stream) override
        {
            // Extract filename and put it to a temporary directory.
            std::string disp;
            NameValueCollection params;
            if (header.has("Content-Disposition"))
            {
                std::string cd = header.get("Content-Disposition");
                MessageHeader::splitParameters(cd, disp, params);
            }

            if (!params.has("filename"))
                return;
            if (params.get("filename").empty())
                return;

            auto tempPath = Path::forDirectory(
                    TemporaryFile::tempName() + "/");
            File(tempPath).createDirectories();
            // Prevent user inputting anything funny here.
            // A "filename" should always be a filename, not a path
            const Path filenameParam(params.get("filename"));
            tempPath.setFileName(filenameParam.getFileName());
            _filename = tempPath.toString();

            // Copy the stream to _filename.
            std::ofstream fileStream;
            fileStream.open(_filename);
            StreamCopier::copyStream(stream, fileStream);
            fileStream.close();

            vars.add("name", _filename);
            fprintf(stderr, "handle part, %s\n", _filename.c_str());
        }
};

MergeODF::MergeODF()
{
#if ENABLE_DEBUG
    ConfigFile = std::string(DEV_DIR) + "/mergeodf.xml";
#else
    ConfigFile = "/etc/ndcodfweb/conf.d/mergeodf/mergeodf.xml";
#endif
    startStamp = std::chrono::steady_clock::now();
    xml_config = new Poco::Util::XMLConfiguration(ConfigFile);

    setLogPath();
    Application::instance().logger().setChannel(channel);
    initSQLDB();
    std::cout << "end construct\n";
    setProgPath(LOOLWSD::LoTemplate);
}

MergeODF::~MergeODF()
{}

// Read XML LOF File
void MergeODF::setLogPath()
{
    Poco::AutoPtr<FileChannel> fileChannel(new FileChannel);
    std::string logFile = xml_config->getString("logging.log_file", "/tmp/mergeodf.log");
    fileChannel->setProperty("path", logFile);
    fileChannel->setProperty("archive", "timestamp");
    Poco::AutoPtr<PatternFormatter> patternFormatter(new PatternFormatter());
    patternFormatter->setProperty("pattern","%Y %m %d %L%H:%M:%S: %t");
    channel = new Poco::FormattingChannel(patternFormatter, fileChannel);
}

void MergeODF::initSQLDB()
{
    //std::cout<<"mergeodf: setlogpath"<<std::endl;
    access_db = new AccessDB();
    access_db->setDbPath();
    std::cout << "end init\n";
    access_db->changeTable();
}

/// api help. yaml&json&json sample(another json)
std::string MergeODF::makeApiJson(std::string which="",
        bool anotherJson,
        bool yaml,
        bool showHead)
{
    std::string jsonstr;

    std::cout << "anotherJson: " << anotherJson << std::endl;
    auto templsts = templLists(false);
    auto it = templsts.begin();
    for (size_t pos = 0; it != templsts.end(); ++it, pos++)
    {
        try
        {
            const auto templfile = *it;
            Parser *parser = new Parser(templfile);
            parser->setOutputFlags(anotherJson, yaml);

            auto endpoint = Poco::Path(templfile).getBaseName();

            if (!which.empty() && endpoint != which)
                continue;


            std::string buf;
            std::string parserResult;
            if (anotherJson)
            {
                buf = "* json 傳遞的 json 資料需以 urlencode(encodeURIComponent) 編碼<br />"
                    "* 圖檔需以 base64 編碼<br />"
                    "* 若以 json 傳參數，則 header 需指定 content-type='application/json'<br /><br />json 範例:<br /><br />";
                buf += Poco::format("{<br />%s}", parser->jjsonVars());
                std::cout << buf << "\n";
            }
            else if (yaml)
                buf = Poco::format(YAMLTEMPL, endpoint, endpoint, parser->yamlVars());
            else
                buf = Poco::format(APITEMPL, endpoint, endpoint, parser->jsonVars());

            delete parser;

            jsonstr += buf;

            if (!which.empty() && endpoint == which)
                break;

            if (pos != templsts.size()-1 && !yaml)
                jsonstr += ",";
        }
        catch (const std::exception & e)
        {
            //...
            std::cout << e.what() << "\n";
        }
    }

    //cout << jsonstr << endl;
    const auto& app = Poco::Util::Application::instance();
    const auto ServerName = app.config().getString("server_name");
    // add header
    if (showHead && !anotherJson)
    {
        std::string read;
        if (yaml)
        {
            read = Poco::format(YAMLTEMPLH, ServerName, jsonstr);
            StringTokenizer tmp(read, "\n");
            read = "";
            std::cout << tmp.count() <<"\n";
            for (unsigned int i = 0; i<tmp.count() ; i++)
            {
                if(!tmp[i].empty() && tmp[i].size()>12)
                    read += tmp[i].substr(12) + "\n";
            }
        }
        else
            read = Poco::format(TEMPLH, ServerName, jsonstr);
        return read;
    }

    return jsonstr;
}

/// validate if match rest uri: /accessTime
std::string MergeODF::isMergeToQueryAccessTime(std::string uri)
{
    auto templsts = templLists(true);
    for (auto it = templsts.begin(); it != templsts.end(); ++it)
    {
        const auto endpoint = *it;
        if (uri == (resturl + endpoint + "/accessTime"))
            return endpoint;
    }
    return "";
}

/// validate if match rest uri
std::string MergeODF::isMergeToUri(std::string uri, bool forHelp,
        bool anotherJson, bool yaml)
{
    auto templsts = templLists(true);
    for (auto it = templsts.begin(); it != templsts.end(); ++it)
    {
        const auto endpoint = *it;
        if (forHelp)
        {
            if (uri == (resturl + endpoint + "/api"))
                return endpoint;
            if (uri == (resturl + endpoint + "/yaml") && yaml)
                return endpoint;
            if (uri == (resturl + endpoint + "/json") && anotherJson)
                return endpoint;
        }
        else
        {
            //std::cout<<uri<<std::endl;
            if (uri == (resturl + endpoint) ||
                    uri == (resturl + endpoint + "?outputPDF=false"))
                return endpoint;
            if (uri == (resturl + endpoint + "?outputPDF") ||
                    uri == (resturl + endpoint + "?outputPDF=") ||
                    uri == (resturl + endpoint + "?outputPDF=true"))
                return "pdf";
        }
    }
    return "";
}

/// validate if match rest uri for /mergeto/[doc]/api
/// anotherJson=true for /mergeto/[doc]/json
std::string MergeODF::isMergeToHelpUri(std::string uri,
        bool anotherJson,
        bool yaml)
{
    return isMergeToUri(uri, true, anotherJson, yaml);
}

// document's extname, used for content-disposition
std::string MergeODF::getDocExt()
{
    if (mimetype == "application/vnd.oasis.opendocument.text")
        return "odt";
    if (mimetype == "application/vnd.oasis.opendocument.spreadsheet")
        return "ods";
    return "odt";
}

/// json: 關鍵字轉小寫，但以quote包起來的字串不處理
std::string MergeODF::keyword2Lower(std::string in, std::string keyword)
{
    RegularExpression re(keyword, RegularExpression::RE_CASELESS);
    RegularExpression::Match match;

    auto matchSize = re.match(in, 0, match);

    //std::cout << match.size()<<std::endl;
    while (matchSize > 0)
    {
        // @TODO: add check for "   null   "
        if (in[match.offset - 1] != '"' &&
                in[match.offset + keyword.size()] != '"')
        {
            for(unsigned idx = 0; idx < keyword.size(); idx ++)
                in[match.offset + idx] = keyword[idx];
        }
        matchSize = re.match(in, match.offset + match.length, match);
    }
    return in;
}

/// 解析表單陣列： 詳細資料[0][姓名] => 詳細資料:姓名
Object::Ptr MergeODF::parseArray2Form(HTMLForm &form)
{
    // {"詳細資料": [ {"姓名": ""} ]}
    std::map <std::string,
        std::vector<std::map<std::string, std::string>>
            > grpNames;
    // 詳細資料[0][姓名] => {"詳細資料": [ {"姓名": ""} ]}
    Object::Ptr formJson = new Object();

    for (auto iterator = form.begin();
            iterator != form.end();
            iterator ++)
    {
        const auto varname = iterator->first;
        const auto value = iterator->second;
        //std::cout <<"===>"<< varname << std::endl;

        auto res = "^([^\\]\\[]*)\\[([^\\]\\[]*)\\]\\[([^\\]\\[]*)\\]$";
        RegularExpression re(res);
        RegularExpression::MatchVec posVec;
        re.match(varname, 0, posVec);
        //std::cout<<"reg size:"<<posVec.size()<<std::endl;
        if (posVec.empty())
        {
            formJson->set(varname, Poco::Dynamic::Var(value));
            continue;
        }

        const auto grpname = varname.substr(posVec[1].offset,
                posVec[1].length);
        const auto grpidxRaw = varname.substr(posVec[2].offset,
                posVec[2].length);
        const auto grpkey = varname.substr(posVec[3].offset,
                posVec[3].length);
        const int grpidx = std::stoi(grpidxRaw);

        //std::vector<std::map<std::string, std::string>> dummy;
        if (grpNames.find(grpname) == grpNames.end())
        {  // default array
            std::vector<std::map<std::string, std::string>> dummy;
            grpNames[grpname] = dummy;
        }

        // 詳細資料[n][姓名]: n to resize
        // n 有可能 1, 3, 2, 6 不照順序, 這裡以 n 當作 resize 依據
        // 就可以調整陣列大小了
        if (grpNames[grpname].size() < (unsigned)(grpidx + 1))
            grpNames[grpname].resize(grpidx + 1);
        //std::cout<<"grpidx:"<<grpidx<<std::endl;
        grpNames[grpname].at(grpidx)[grpkey] = value;
    }
    // {"詳細資料": [ {"姓名": ""} ]} => 詳細資料:姓名=value
    //
    for(auto itgrp = grpNames.begin();
            itgrp != grpNames.end();
            itgrp++)
    {
        //std::cout<<"***"<<itgrp->first<<std::endl;
        auto gNames = itgrp->second;
        for(unsigned grpidx = 0; grpidx < gNames.size(); grpidx ++)
        {
            auto names = gNames.at(grpidx);
            Object::Ptr tempData = new Object();
            for(auto itname = names.begin();
                    itname != names.end();
                    itname++)
            {
                tempData->set(itname->first, Poco::Dynamic::Var(itname->second));
            }
            if (names.size() != 0 )
            {
                if(!formJson->has(itgrp->first))
                {
                    Array::Ptr newArr = new Array();
                    formJson->set(itgrp->first, newArr);
                }
                formJson->getArray(itgrp->first)->add(tempData);
            }
        }
    }
    return formJson;
}

/// get api called times
int MergeODF::getApiCallTimes(std::string endpoint)
{
    access_db->setApi(endpoint);
    int accesstimes = access_db->getAccessTimes();
    return accesstimes;
}


/// merge to odf file
std::string MergeODF::doMergeTo(const Poco::Net::HTTPRequest& request,
        Poco::MemoryInputStream& message)
{
    std::string fromPath;
    auto requestUri = Poco::URI(request.getURI());

    Parser *parser = new Parser(requestUri);
    if (!parser->isValid())
    {
        mergeStatus = MergeStatus::TEMPLATE_NOT_FOUND;
        return "";
    }

    MergeODFPartHandler2 handler(fromPath);
    Object::Ptr object;

    if (request.getContentType() == "application/json")
    {
        std::string line, data;
        std::istream &iss(message);
        while (!iss.eof())
        {
            std::getline(iss, line);
            data += line;
        }
        // 解析 request body to json
        std::string jstr = data;

        jstr = keyword2Lower(jstr, "null");
        jstr = keyword2Lower(jstr, "true");
        jstr = keyword2Lower(jstr, "false");
        Poco::JSON::Parser jparser;
        Poco::Dynamic::Var result;

        // Parse data to PocoJSON
        try{
            result = jparser.parse(jstr);
            object = result.extract<Object::Ptr>();
        }
        catch (Poco::Exception& e)
        {
            std::cerr << e.displayText() << std::endl;
            mergeStatus = MergeStatus::JSON_PARSE_ERROR;
            return "";
        }
    }
    else
    {
        HTMLForm form;
        form.setFieldLimit(0);
        form.load(request, message, handler);
        // 資料形式如果是 post HTML Form 上來
        try {
            object = parseArray2Form(form);
        }
        catch (Poco::Exception& e)
        {
            std::cerr << e.displayText() << std::endl;
            mergeStatus = MergeStatus::HTMLFORM_PARSE_ERROR;
            return "";
        }
    }


    // XML 前處理:  遍歷文件的步驟都要在這裡處理,不然隨著文件的內容增加,會導致遍歷時間大量增長

    //把 form 的資料放進 xml 檔案
    auto allVar = parser->scanVarPtr();
    std::list<Element*> singleVar = allVar[0];
    std::list<Element*> groupVar = allVar[1];

    parser->setSingleVar(object, singleVar);
    parser->setGroupVar(object, groupVar);
    const auto zip2 = parser->zipback();
    mimetype = getMimeType(zip2);
    delete parser;
    mergeStatus = MergeStatus::OK;
    return zip2;
}

/// merge status
MergeODF::MergeStatus MergeODF::getMergeStatus(void)
{
    return mergeStatus;
}

/// 轉檔：轉成 odf
std::string MergeODF::outputODF(std::string outfile)
{
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
    lok::Document * lodoc = llo->documentLoad(outfile.c_str(), options);
    if (!lodoc)
    {
        const char * errmsg = llo->getError();
        std::cerr << ": LibreOfficeKit failed to load document (" << errmsg << ")" << std::endl;
        return errmsg;
    }

    outfile = outfile + ".pdf";
    //std::cout << outfile << std::endl;
    if (!lodoc->saveAs(outfile.c_str(), "pdf", options))
    {
        const char * errmsg = llo->getError();
        std::cerr << ": LibreOfficeKit failed to export (" << errmsg << ")" << std::endl;

        delete lodoc;
        return errmsg;
    }

    delete lodoc;
    userprofile.remove(true);

    return outfile;
}

/// response 回傳 api 的呼叫次數
void MergeODF::responseAccessTime(std::weak_ptr<StreamSocket> _socket, std::string endpoint)
{
    int access = getApiCallTimes(endpoint);

    std::ostringstream oss;
    std::string accessTime = "{\"call_time\": " + std::to_string(access) + std::string("}");
    oss << "HTTP/1.1 200 OK\r\n"
        << "Last-Modified: " << Poco::DateTimeFormatter::format(Poco::Timestamp(), Poco::DateTimeFormat::HTTP_FORMAT) << "\r\n"
        << "Access-Control-Allow-Origin: *" << "\r\n"
        << "User-Agent: " << WOPI_AGENT_STRING << "\r\n"
        << "Content-Length: " << accessTime.size() << "\r\n"
        << "Content-Type: application/json; charset=utf-8\r\n"
        << "X-Content-Type-Options: nosniff\r\n"
        << "\r\n"
        << accessTime;

    auto socket = _socket.lock();
    socket->send(oss.str());
    socket->shutdown();
}

/// http://server/lool/mergeodf
/// called by LOOLWSD
void MergeODF::handleMergeTo(std::weak_ptr<StreamSocket> _socket,
        const Poco::Net::HTTPRequest& request,
        Poco::MemoryInputStream& message)
{
    /*
     * Swagger's CORS would send OPTIONS first to check if the server allow CROS, So Check First OPTIONS and allow
     */
    if (request.getMethod() == HTTPRequest::HTTP_OPTIONS)
    {
        quickHttpRes(_socket, HTTPResponse::HTTP_OK, "OK");
        return;
    }

    HTTPResponse response;
    auto socket = _socket.lock();

    response.set("Access-Control-Allow-Origin", "*");
    response.set("Access-Control-Allow-Methods", "POST, OPTIONS");
    response.set("Access-Control-Allow-Headers",
            "Origin, X-Requested-With, Content-Type, Accept");

    std::string zip2;
    auto endpointPath = Poco::URI(request.getURI()).getPath();
    auto endpoint = Poco::Path(endpointPath).getBaseName();
    access_db->setApi(endpoint);
    logger().notice(endpoint + ": start process");
    access_db->updateAccessTimes();
    const auto toPdf = isMergeToUri(request.getURI()) == "pdf";

    try{
        logger().notice(endpoint + ": start merge");
        zip2 = doMergeTo(request, message);
    }
    catch (const std::exception & e)
    {
        logger().notice(endpoint + ": 轉換失敗");

        quickHttpRes(_socket, HTTPResponse::HTTP_INTERNAL_SERVER_ERROR,
                "轉換失敗");
        return;
    }

    if (getMergeStatus() != MergeStatus::OK)
    {
        if (mergeStatus == MergeStatus::JSON_PARSE_ERROR)
        {
            logger().notice(endpoint + ": Json 格式錯誤");
            quickHttpRes(_socket, HTTPResponse::HTTP_BAD_REQUEST,
                    "Json 格式錯誤");
            log(false, endpoint, "Json 格式錯誤", toPdf);
        }
        else if (mergeStatus == MergeStatus::HTMLFORM_PARSE_ERROR)
        {
            logger().notice(endpoint + ": form 格式錯誤");
            quickHttpRes(_socket, HTTPResponse::HTTP_BAD_REQUEST,
                    "form 格式錯誤");
            log(false, endpoint, "form 格式錯誤", toPdf);
        }
        return;
    }
    logger().notice(endpoint + ": merge ok");

    auto mimeType = getMimeType(zip2);

    auto docExt = !toPdf ? getDocExt() : "pdf";
    response.set("Content-Disposition",
            "attachment; filename=\"" + endpoint + "."+ docExt +"\"");

    if (!toPdf)
    {
        HttpHelper::sendFileAndShutdown(socket, zip2, mimeType, &response);

        log(true, endpoint, "Success", toPdf);
        return;
    }

    logger().notice(endpoint + ": start convert to pdf");

    auto zip2pdf = outputODF(zip2);
    if (zip2pdf.empty() || !Poco::File(zip2pdf).exists())
    {
        logger().notice(endpoint + ": mergeing to pdf error");


        quickHttpRes(_socket, HTTPResponse::HTTP_SERVICE_UNAVAILABLE,
             "輸出 PDF 錯誤");
        log(false, endpoint, "converting to pdf error", toPdf);
        return;
    }

    logger().notice(endpoint + ": convert to pdf: done");
    log(true, endpoint, "converting to pdf success", toPdf);

    HttpHelper::sendFileAndShutdown(socket, zip2pdf, mimeType, &response);
}


void MergeODF::handleAPIHelp(std::weak_ptr<StreamSocket> _socket,
        bool showMerge,
        std::string mergeEndPoint,
        bool anotherJson,
        bool yaml)
{
    const auto& app = Poco::Util::Application::instance();
    const auto ServerName = app.config().getString("server_name");
#if ENABLE_DEBUG
    std::cout << "Skip checking the server_name...\n";
#else
    // 檢查是否有填 server_name << restful client 依據此作為呼叫之 url
    // url 帶入 TEMPL 之 "host"
    if (app.config().getString("server_name").empty())
    {
        quickHttpRes(_socket, HTTPResponse::HTTP_SERVICE_UNAVAILABLE,
                "server_name 無指定");
        return;
    }
#endif
    const std::string TEMPL = R"MULTILINE(
        {
            "swagger": "2.0",
            "info": {
                "version": "v1",
                "title": "轉檔管理 & ODF 報表 API",
                "description": ""
            },
            "host": "%s",
            "paths": {
                %s
            }
        }
        )MULTILINE";

        const std::string HELP_YAMLTEMPL = R"MULTILINE(
        swagger: '2.0'
        info:
            version: v1
            title: 轉檔管理 & ODF 報表 API
            description: ''
        host: %s
        paths:%s)MULTILINE";

    std::string paths = "";
    bool showHead = !(false && showMerge);
    std::cout << "checkpoint before make api json\n";
    if (showMerge)
    {
        auto merge = makeApiJson(mergeEndPoint,
                anotherJson, yaml, showHead);
        if (!paths.empty() && !merge.empty() && !yaml)
            paths += ",";
        paths += merge;
    }
    if (!showHead && !yaml)
        paths = Poco::format(TEMPL, ServerName, paths);
    if (!showHead && yaml)
        paths = Poco::format(HELP_YAMLTEMPL, ServerName, paths);

    if (paths.empty())
    {
        auto socket = _socket.lock();
        socket->shutdown();
        return;
    }
    std::string read = paths;

    auto mediaType = !anotherJson && !yaml ? "application/json" :
        !yaml ? "text/html; charset=utf-8" :
        "text/plain; charset=utf-8";

    // TODO: Refactor this to some common handler.
    std::ostringstream oss;
    oss << "HTTP/1.1 200 OK\r\n"
        << "Last-Modified: " << Poco::DateTimeFormatter::format(Poco::Timestamp(), Poco::DateTimeFormat::HTTP_FORMAT) << "\r\n"
        << "Access-Control-Allow-Origin: *" << "\r\n"
        << "User-Agent: " << WOPI_AGENT_STRING << "\r\n"
        << "Content-Length: " << read.size() << "\r\n"
        << "Content-Type: " << mediaType << "\r\n"
        << "X-Content-Type-Options: nosniff\r\n"
        << "\r\n"
        << read;

    auto socket = _socket.lock();
    socket->send(oss.str());
    socket->shutdown();
}


/*
 * FileServer Implement
 */
void MergeODF::uploadFile(std::weak_ptr<StreamSocket> _socket,
        MemoryInputStream& message,
        HTTPRequest& request)
{
    std::cout << "upload file\n" ;
    ManageFilePartHandler handler(std::string(""));
    HTMLForm form(request, message, handler);
    if (handler.vars.has("name"))
    {
        auto filedb = FileDB();
        if (filedb.setFile(form))
        {
            std::string tempPath = handler.vars.get("name");
            Poco::File tempFile(tempPath);
            tempFile.copyTo(template_dir + form.get("endpt") + "." + form.get("extname"));
            tempFile.remove();
            quickHttpRes(_socket, HTTPResponse::HTTP_OK, "Upload Success");
        }
        else
        {
            quickHttpRes(_socket, HTTPResponse::HTTP_SERVICE_UNAVAILABLE, "endpt already exists");
        }
    }
    else
    {
        quickHttpRes(_socket, HTTPResponse::HTTP_SERVICE_UNAVAILABLE, "fail to save file to server");
    }
}

void MergeODF::downloadFile(std::weak_ptr<StreamSocket> _socket,
        MemoryInputStream& message,
        HTTPRequest& request)
{
    HTMLForm form(request, message);
    std::string endpt = form.get("endpt", "");
    std::string extname = form.get("extname", "");
    if (endpt.empty())
    {
        quickHttpRes(_socket, HTTPResponse::HTTP_SERVICE_UNAVAILABLE, "No file specify for download");
        return;
    }
    FileDB filedb;
    std::string filePath = template_dir + filedb.getFile(endpt);
    Poco::File targetFile(filePath);
    if (targetFile.exists())
    {
        HTTPResponse response;
        auto socket = _socket.lock();

        response.set("Access-Control-Allow-Origin", "*");
        response.set("Access-Control-Allow-Methods", "POST, OPTIONS");
        response.set("Access-Control-Allow-Headers",
                "Origin, X-Requested-With, Content-Type, Accept");
        response.set("Content-Disposition",
                "attachment; filename=\"" + endpt + "."+ extname+"\"");
        std::string mimeType = getMimeType(filePath);
        HttpHelper::sendFileAndShutdown(socket, filePath, mimeType, &response);
    }
    else
        quickHttpRes(_socket, HTTPResponse::HTTP_SERVICE_UNAVAILABLE, "No such file, please confirm");

}

void MergeODF::deleteFile(std::weak_ptr<StreamSocket> _socket,
        MemoryInputStream& message,
        HTTPRequest& request)
{
    HTMLForm form(request, message);
    std::string endpt = form.get("endpt", "");
    std::string extname = form.get("extname", "");
    if (endpt == "")
    {
        quickHttpRes(_socket, HTTPResponse::HTTP_SERVICE_UNAVAILABLE, "No endpt provide");
        return;
    }
    Poco::File targetFile(template_dir + endpt + "." + extname);
    if (targetFile.exists())
    {
        targetFile.remove();
        auto filedb = FileDB();
        filedb.delFile(endpt);
        quickHttpRes(_socket, HTTPResponse::HTTP_OK, "delete success");
    }
    else
    {
        quickHttpRes(_socket, HTTPResponse::HTTP_SERVICE_UNAVAILABLE, "File not exist");
    }
}

void MergeODF::updateFile(std::weak_ptr<StreamSocket> _socket,
        MemoryInputStream& message,
        HTTPRequest& request)
{
    ManageFilePartHandler handler(std::string(""));
    HTMLForm form(request, message, handler);
    if (handler.vars.has("name"))
    {
        auto filedb = FileDB();
        std::string endpt = form.get("endpt", "");
        std::string extname = form.get("extname", "");
        std::string oldFile = filedb.getFile(endpt);
        std::cout << "oldFile: " << oldFile << std::endl;
        Poco::File targetFile(template_dir + oldFile);
        if (targetFile.exists())
        {
            targetFile.remove();
            filedb.delFile(endpt);
            filedb.setFile(form);
            Poco::File tempFile(handler.vars.get("name"));
            tempFile.copyTo(template_dir + endpt + "." + extname);
            tempFile.remove();
            quickHttpRes(_socket, HTTPResponse::HTTP_OK, "Update success");
        }
        else
        {
            quickHttpRes(_socket, HTTPResponse::HTTP_SERVICE_UNAVAILABLE, "No such file, please contact admin");
        }
    }
    else
    {
        quickHttpRes(_socket, HTTPResponse::HTTP_SERVICE_UNAVAILABLE,"No file provide");
    }
}
/*
 * END OF FileServer
 */

//LogDB

void MergeODF::log(bool success,
                std::string endpt,
                std::string e_msg,
                bool toPDF)
{
    const auto timeSinceStartMs =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - startStamp).count();
    std::string timeSince = std::to_string(timeSinceStartMs * 0.001);
    std::cout << e_msg << "\n";
    auto filedb = FileDB();
    auto filename = filedb.getDocname(endpt);
    // Store in SQLite
    LogDB log_db;
    StringTokenizer file_tokens(filename, ".");
    Poco::Timestamp tstmp;
    std::string datetime = Poco::DateTimeFormatter::format(tstmp, "%Y-%n-%e %H:%M:%S");
    log_db.addRecord(success, toPDF, reqClientAddress,
            file_tokens[0], file_tokens[1],
            timeSince, datetime);
}


/*
 * OXOOLMODULE interface Implement
 */
std::string MergeODF::getHTMLFile(std::string fileName)
{
    std::string filePath = "";
#if ENABLE_DEBUG
    std::string dev_path(DEV_DIR);
    filePath = dev_path + "/html/";
#else
    filePath = "/var/lib/oxool/mergeodf/html/";
#endif
    filePath += fileName;
    std::cout << "filePath: " << filePath << std::endl;
    return filePath;
}


void MergeODF::handleRequest(std::weak_ptr<StreamSocket> _socket,
        MemoryInputStream& message,
        HTTPRequest& request,
        SocketDisposition& disposition)
{
    StringTokenizer clientaddress_tokens(_socket.lock()->clientAddress(), ":");
    reqClientAddress = clientaddress_tokens[clientaddress_tokens.count()-1];
    Process::PID pid = fork();
    if (pid < 0)
    {
        quickHttpRes(_socket, HTTPResponse::HTTP_INTERNAL_SERVER_ERROR, "轉換失敗");
    }
    else if (pid == 0)
    {
        try{
            std::cout << "Current pid: " << getpid() << "\n";
            if(false)
                disposition.setClosed();
            Poco::URI requestUri(request.getURI());
            std::vector<std::string> reqPathSegs;
            requestUri.getPathSegments(reqPathSegs);
            std::string method = request.getMethod();
            std::string URI = request.getURI();
#if ENABLE_DEBUG
            template_dir = std::string(DEV_DIR) + "/runTimeData/mergeodf/";
#else
            auto mergeodfconf = new Poco::Util::XMLConfiguration("/etc/ndcodfweb/conf.d/mergeodf/mergeodf.xml");
            template_dir = mergeodfconf->getString("template.dir_path", "");
#endif
            if (method == HTTPRequest::HTTP_GET && reqPathSegs.size()==2)
            {
                quickHttpRes(_socket, HTTPResponse::HTTP_OK);
            }
            else if (method == HTTPRequest::HTTP_GET &&
                    URI == "/lool/mergeodf/api")
            {  // /lool/[mergeodf]/api
                bool showMerge = request.getURI() == "/lool/mergeodf/api";
                handleAPIHelp(_socket, showMerge);
                std::cout << "handle help end\n";
            }
            else if (method == HTTPRequest::HTTP_GET &&
                    URI == "/lool/mergeodf/yaml")
            {  // /lool/[mergeodf]/yaml
                bool showMerge = request.getURI() == "/lool/mergeodf/yaml";
                handleAPIHelp(_socket, showMerge, "", false, true);
            }
            else if (method == HTTPRequest::HTTP_GET &&
                    !isMergeToQueryAccessTime(URI).empty())
            {  // /lool/mergeodf/doc_id/accessTime
                auto endpoint = isMergeToQueryAccessTime(request.getURI());
                responseAccessTime(_socket, endpoint);
            }
            else if (method == HTTPRequest::HTTP_GET &&
                    !isMergeToHelpUri(URI).empty())
            {  // /lool/mergeodf/doc_id/api
                auto endpoint = isMergeToHelpUri(request.getURI());
                handleAPIHelp(_socket, true, endpoint);
            }
            else if (method == HTTPRequest::HTTP_GET &&
                    !isMergeToHelpUri(URI, false, true).empty())
            {  // /lool/mergeodf/doc_id/yaml
                auto endpoint = isMergeToHelpUri(request.getURI(), false, true);
                handleAPIHelp(_socket, true, endpoint, false, true);
            }
            else if (method == HTTPRequest::HTTP_GET &&
                    !isMergeToHelpUri(URI, true).empty())
            {  // for another json help: /lool/mergeodf/doc_id/json
                auto endpoint = isMergeToHelpUri(request.getURI(), true);
                handleAPIHelp(_socket, true, endpoint, true);
            }
            else if ((method == HTTPRequest::HTTP_POST ||
                        method == HTTPRequest::HTTP_OPTIONS) &&
                    !isMergeToUri(URI).empty())
            {  // /lool/mergeodf/doc_id
                handleMergeTo(_socket, request, message);
            }
            else if (method == HTTPRequest::HTTP_POST &&  URI == "/lool/mergeodf/upload")
            {
                uploadFile(_socket, message, request);
            }
            else if (method == HTTPRequest::HTTP_POST &&  URI == "/lool/mergeodf/delete")
            {
                deleteFile(_socket, message, request);
            }
            else if (method == HTTPRequest::HTTP_POST &&  URI == "/lool/mergeodf/update")
            {
                updateFile(_socket, message, request);
            }
            else if (method == HTTPRequest::HTTP_POST &&  URI == "/lool/mergeodf/download")
            {
                downloadFile(_socket, message, request);
            }
            else
            {
                quickHttpRes(_socket, HTTPResponse::HTTP_NOT_FOUND, "無此 API");
            }
        }
        catch(const std::exception &e)
        {
            std::cout << e.what() << std::endl;
            logger().notice("[Exception]" + std::string(e.what()));
            quickHttpRes(_socket, HTTPResponse::HTTP_INTERNAL_SERVER_ERROR, "轉換失敗");
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
        std::cout << "mergeodf shutdown \n";
        exit_application = true;
    }
    else
    {
        std::cout << "call from parent" << std::endl;
    }
}



std::string MergeODF::handleAdmin(std::string command)
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

extern "C" MergeODF *create_object()
{
    return new MergeODF;
}

extern "C" void destroy_object(MergeODF *mergeodf)
{
    delete mergeodf;
}