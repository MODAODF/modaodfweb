#include "mergeodf_logging_db.h"
#include <stdio.h>
#include "Log.hpp"

#include <Poco/Data/Session.h>
#include <Poco/Data/RecordSet.h>
#include <Poco/Data/SQLite/Connector.h>
#include <Poco/Util/XMLConfiguration.h>
#include <Poco/StringTokenizer.h>

#include <sqlite3.h>
#include <Poco/Data/SQLite/Utility.h>

using Poco::Data::Statement;
using Poco::Data::RecordSet;
using Poco::Data::SQLite::Utility;
using namespace Poco::Data::Keywords;
using Poco::StringTokenizer;

LogDB::LogDB()
{
    Poco::Data::SQLite::Connector::registerConnector();
    setDbPath();

    // 初始化 Database
    Poco::Data::Session session("SQLite", dbfile);
    Statement select(session);
    std::string init_sql = R"MULTILINE(
        CREATE TABLE IF NOT EXISTS logging (
            rec_id      integer primary key,
            status      text NOT NULL,
            to_pdf      text NOT NULL,
            source_ip   text NOT NULL,
            file_name   text NOT NULL,
            file_ext    text NOT NULL,
            cost        text NOT NULL,
            timestamp   text NOT NULL
        )
        )MULTILINE";
    select << init_sql;
    while (!select.done())
    {
        select.execute();
        break;
    }
    select.reset(session);
    session.close();
}
LogDB::~LogDB() {
    Poco::Data::SQLite::Connector::unregisterConnector();
}

/// 從設定檔取得資料庫檔案位置名稱 & timeout
void LogDB::setDbPath()
{
    // 如果沒有跑系統，會讀取專案底下的 runTimeData/converter.sqlite 來確保程式執行
#if ENABLE_DEBUG
    dbfile = std::string(DEV_DIR) + "/runTimeData/mergeodf.sqlite";
#else
    auto mergeodfConf = new Poco::Util::XMLConfiguration("/etc/ndcodfweb/conf.d/mergeodf/mergeodf.xml");
    dbfile = mergeodfConf->getString("database.db_path", "");
#endif
}

 // record format: "status,source_ip,docname,extname,endpt,timestamp"
void LogDB::addRecord(int status, int to_pdf,
                      std::string source_ip,
                      std::string file_name,
                      std::string file_ext,
                      std::string cost,
                      std::string timestamp)
{
    Poco::Data::Session session("SQLite", dbfile);
    Statement select(session);
    std::string init_sql = "insert into logging (status, to_pdf, source_ip, file_name, file_ext, cost, timestamp) values (?,?,?,?,?,?,?)";
    select << init_sql,
           use(status),
           use(to_pdf),
           use(source_ip),
           use(file_name),
           use(file_ext),
           use(cost),
           use(timestamp);
    while (!select.done())
    {
        select.execute();
        break;
    }
    select.reset(session);
    session.close();
}

std::string LogDB::getRecord(std::string rec_id)
{
    Poco::Data::Session session("SQLite", dbfile);

    Statement select(session);
    std::string line;

    select << "SELECT rec_id, status, to_pdf, source_ip, file_name, file_ext, cost, timestamp FROM logging where rec_id >?",use(rec_id);
    RecordSet rs(select);

    std::vector<std::string> data;
    std::string result;
    try
    {
        while (!select.done())
        {
            select.execute();
            bool more = rs.moveFirst();
            while (more)
            {
                line = R"MULTILINE(
                    {
                        "rec_id"      : "%s",
                        "status"      : "%s",
                        "to_pdf"      : "%s",
                        "source_ip"   : "%s",
                        "file_name"   : "%s",
                        "file_ext"    : "%s",
                        "cost"        : "%s",
                        "timestamp"   : "%s"
                    }
                    )MULTILINE";
                for(unsigned int it = 0; it < rs.columnCount(); it++)
                {
                    Poco::RegularExpression string_rule("\%s");
                    string_rule.subst(line, rs[it].convert<std::string>());
                }
                more = rs.moveNext();
                data.push_back(line);
            }
        }
        result = "[";
        for(auto it = data.begin();it!=data.end();)
        {
            result += *it;
            it++;
            if(it!=data.end())
                result+=",";
        }
        result += "]";
    }
    catch (Poco::Exception& e)
    {
        LOG_ERR("sql executed failed: " << e.displayText() <<
                ", SQL: " << select.toString());
        select.reset(session);
        rs.reset(session);
        session.close();
        return "[]";
    }
    select.reset(session);
    rs.reset(session);
    session.close();
    return result;
}
