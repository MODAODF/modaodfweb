#include "mergeodf_access_db.h"
#include "Log.hpp"

#include <Poco/Data/Session.h>
#include <Poco/Data/RecordSet.h>
#include <Poco/Data/SQLite/Connector.h>
#include <Poco/Util/XMLConfiguration.h>

#include <sqlite3.h>
#include <Poco/Data/SQLite/Utility.h>

using Poco::Data::Statement;
using Poco::Data::RecordSet;
using Poco::Data::SQLite::Utility;
using namespace Poco::Data::Keywords;


AccessDB::AccessDB()
{
    Poco::Data::SQLite::Connector::registerConnector();
    // 初始化 Database
    Poco::Data::Session session("SQLite", dbfile);
    Statement select(session);
    std::string init_sql = R"MULTILINE(
    CREATE TABLE merge_templates (
            rec_id int ,
            uid int ,
            cid int ,
            title TEXT,
            desc text ,
            endpt text UNIQUE,
            docname text ,
            extname text ,
            uptime text
            );
    CREATE TABLE logging (
                rec_id      integer primary key,
                status      text NOT NULL,
                to_pdf      text NOT NULL,
                source_ip   text NOT NULL,
                file_name   text NOT NULL,
                file_ext    text NOT NULL,
                cost        text NOT NULL,
                timestamp   text NOT NULL
            );
    CREATE TABLE summary (api TEXT PRIMARY KEY NOT NULL UNIQUE, accessTimes INTEGER NOT NULL);
    CREATE TABLE access (api text not null, status text not null, ts timestamp DEFAULT CURRENT_TIMESTAMP);
    CREATE INDEX query on access (api, status);
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
AccessDB::~AccessDB() {
    Poco::Data::SQLite::Connector::unregisterConnector();
}

/// 從設定檔取得資料庫檔案位置名稱 & timeout
void AccessDB::setDbPath()
{
    // 如果沒有跑系統，會讀取專案底下的 runTimeData/mergeodf.sqlite 來確保程式執行
#if ENABLE_DEBUG
    dbfile = std::string(DEV_DIR) + "/runTimeData/mergeodf.sqlite";
#else
    //dbfile = MODULES_DATA_DIR + "mergeodf/mergeodf.sqlite";
    auto mergeodfConf = new Poco::Util::XMLConfiguration("/etc/modaodfweb/conf.d/mergeodf/mergeodf.xml");
    dbfile = mergeodfConf->getString("database.db_path", "");
#endif
    std::cout<<"log_db: "<< dbfile << std::endl;
}

// This function is for old ndcodfapi to modaodfweb-2
void AccessDB::changeTable()
{
    Poco::Data::Session session("SQLite", dbfile);
    Statement select(session);
    try
    {
        select << "CREATE TABLE IF NOT EXISTS summary (api TEXT PRIMARY KEY NOT NULL UNIQUE, accessTimes INTEGER NOT NULL)";
        while (!select.done())
        {
            select.execute();
            break;
        }
        select.reset(session);

        // Start transform old table access to summary
        std::vector<std::string> apiName;
        Statement select2(session);
        select2 << "select DISTINCT api FROM access", into(apiName);
        while (!select2.done())
        {
            select2.execute();
            break;
        }
        select2.reset(session);

        int access = 0;
        for(auto it = apiName.begin(); it != apiName.end(); it++)
        {
            std::string currentAPI = *it;
            Statement countStmt(session);
            countStmt << "select count(*) FROM access where api=? and status='start'", into(access), use(currentAPI);
            while (!countStmt.done())
            {
                countStmt.execute();
                break;
            }
            countStmt.reset(session);

            Statement replaceStmt(session);
            replaceStmt << "replace into summary (api, accessTimes) values (?, ?)", use(currentAPI), use(access);
            while (!replaceStmt.done())
            {
                replaceStmt.execute();
                break;
            }
            replaceStmt.reset(session);

            Statement delStmt(session);
            delStmt << "Delete FROM access where api=?", use(currentAPI);
            while (!delStmt.done())
            {
                delStmt.execute();
                break;
            }
            delStmt.reset(session);
        }

        Statement vacuum(session);
        vacuum << "vacuum";
        while (!vacuum.done())
        {
            vacuum.execute();
            break;
        }
        vacuum.reset(session);

        session.close();
    }
    catch (const std::exception & e)
    {
        std::cout << e.what() << "\n";
    }


}

void AccessDB::updateAccessTimes()
{
    Poco::Data::Session session("sqlite", dbfile);


    Statement checkStmt(session);
    int value=-1;
    checkStmt << "select accessTimes from summary where api=?", use(api), into(value);
    while (!checkStmt.done())
    {
        checkStmt.execute();
        break;
    }
    checkStmt.reset(session);

    if (value == -1)
    {
        Statement addNew(session);
        addNew << "insert into summary values (?, 1)", use(api);
        while (!addNew.done())
        {
            addNew.execute();
            break;
        }
        addNew.reset(session);
    }
    else
    {
        Statement updateStmt(session);
        updateStmt << "Update summary set accessTimes = accessTimes + 1 where api=?", use(api);
        while (!updateStmt.done())
        {
            updateStmt.execute();
            break;
        }
        updateStmt.reset(session);

    }

    session.close();
}

/// 傳回某個　api 的呼叫次數
int AccessDB::getAccessTimes()
{
    int access = 0;

    auto session = Poco::Data::Session("SQLite", dbfile);
    Statement select(session);
    select << "select accesstimes from summary where api=?", into(access), use(api);
    while (!select.done())
    {
        select.execute();
        break;
    }
    select.reset(session);
    session.close();
    return access;
}
