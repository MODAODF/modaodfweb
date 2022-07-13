#include "tbl2sc_logging_db.h"
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
            source_ip   text NOT NULL,
            title       text NOT NULL,
            cost        text NOT NULL,
            timestamp   text NOT NULL,
            msg         text NOT NULL
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

/// 從設定檔取得資料庫檔案位置名稱
void LogDB::setDbPath()
{
    auto tbl2scConf = new Poco::Util::XMLConfiguration(LOOLWSD_MODULE_CONFIG_DIR "/tbl2sc.xml");
    dbfile = tbl2scConf->getString("database.db_path", "");
}

 // record format: "status,source_ip,docname,extname,endpt,timestamp"
void LogDB::addRecord(int status,
                      std::string source_ip,
                      std::string title,
                      std::string cost,
                      std::string timestamp,
                      std::string msg)
{
    Poco::Data::Session session("SQLite", dbfile);
    Statement select(session);
    std::string init_sql = "insert into logging (status, source_ip, title, cost, timestamp, msg) values (?,?,?,?,?,?)";
    select << init_sql,
           use(status),
           use(source_ip),
           use(title),
           use(cost),
           use(timestamp),
           use(msg);
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

    select << "SELECT rec_id, status, source_ip, title, cost, timestamp, msg FROM logging where rec_id >?",use(rec_id);
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
                        "source_ip"   : "%s",
                        "title"       : "%s",
                        "cost"        : "%s",
                        "timestamp"   : "%s",
                        "msg"   : "%s"
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
        session.close();
        return "[]";
    }
    select.reset(session);
    rs.reset(session);
    session.close();
    return result;
}
