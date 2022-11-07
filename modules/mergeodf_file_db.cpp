#include <string>
#include <iostream>
#include "mergeodf_file_db.h"

#include <Poco/Data/Session.h>
#include <Poco/Data/RecordSet.h>
#include <Poco/Data/SQLite/Connector.h>
#include <Poco/Data/Session.h>
#include <Poco/Util/XMLConfiguration.h>
#include <Poco/Timestamp.h>
#include <Poco/Data/Statement.h>
#include <Poco/Exception.h>
#include <Poco/UUIDGenerator.h>
#include <Poco/StringTokenizer.h>
#include <Poco/Net/NetworkInterface.h>
#include <Poco/Net/IPAddress.h>
#include <Poco/Net/DNS.h>


using Poco::Data::Statement;
using Poco::StringTokenizer;
using Poco::Data::RecordSet;
using namespace Poco::Data::Keywords;

FileDB::FileDB() {
    Poco::Data::SQLite::Connector::registerConnector();

    setDbPath();

    // 初始化 Database
    Poco::Data::Session session("SQLite", dbfile);
    Statement select(session);
    std::string init_sql = R"MULTILINE(
        CREATE TABLE IF NOT EXISTS merge_templates (
        rec_id int ,
        uid int ,
        cid int ,
        title TEXT,
        desc text ,
        endpt text UNIQUE,
        docname text ,
        extname text ,
        uptime text
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
FileDB::~FileDB() {
    Poco::Data::SQLite::Connector::unregisterConnector();
}


void FileDB::setDbPath()
{
#if ENABLE_DEBUG
    dbfile = std::string(DEV_DIR) + "/runTimeData/mergeodf.sqlite";
#else
    auto previewConf = new Poco::Util::XMLConfiguration("/etc/modaodfweb/conf.d/mergeodf/mergeodf.xml");
    dbfile = previewConf->getString("template.db_path", "");
#endif
}



/// @TODO: duplicate uuid?
std::string FileDB::newkey()
{
    Poco::Data::Session session("SQLite", dbfile);
    int key_old = 0;
    int re_count = 0;
    std::string key;
    while(true)
    {
        Poco::UUIDGenerator& gen = Poco::UUIDGenerator::defaultGenerator();
        key = gen.create().toString();
        Statement check_uuid(session);
        check_uuid << "select COUNT(uuid) from keylist where uuid=?;",
               use(key), into(key_old);
        while (!check_uuid.done())
        {
            check_uuid.execute();
        }
        if(key_old == 0)
        {
            check_uuid.reset(session);
            try{
                Statement insert(session);
                insert << "INSERT INTO keylist (uuid, expires) VALUES (?, strftime('%s', 'now'))",
                       use(key), now;
                while (!insert.done())
                {
                    insert.execute();
                }
                insert.reset(session);
                session.close();
                break;
            }
            catch(std::exception &e)
            {
                std::cout << e.what() << std::endl;
            }
        }
        if(re_count > 30)
        {
            check_uuid.reset(session);
            throw "資料表 keylist 的 uuid 過多,請清除";
        }
        check_uuid.reset(session);
        re_count++;
    }
    return key;
}

bool FileDB::setFile(Poco::Net::HTMLForm &form)
{
    Poco::Data::Session session("SQLite", dbfile);
    Statement select(session);
    std::string init_sql = "Insert into merge_templates values (?,?,?,?,?,?,?,?,?)";
    std::string rec_id  = form.get("rec_id", "");
    std::string uid     = form.get("uid", "");
    std::string cid     = form.get("cid", "");
    std::string title   = form.get("title", "");
    std::string desc    = form.get("desc", "");
    std::string endpt   = form.get("endpt", "");
    std::string docname = form.get("docname", "");
    std::string extname = form.get("extname", "");
    std::string uptime  = form.get("uptime", "");
    select << init_sql, use(rec_id),
                        use(uid),
                        use(cid),
                        use(title),
                        use(desc),
                        use(endpt),
                        use(docname),
                        use(extname),
                        use(uptime);
    try
    {
        while (!select.done())
        {
            select.execute();
            break;
        }
        select.reset(session);
        session.close();
        return true;
    }
    catch (std::exception &e)
    {
        std::cout << e.what() << std::endl;
        return false;
    }
}

void FileDB::delFile(std::string endpt)
{
    Poco::Data::Session session("SQLite", dbfile);
    Statement select(session);
    std::string del_sql = "Delete from merge_templates where endpt=?";
    select << del_sql, use(endpt);
    while (!select.done())
    {
        select.execute();
        break;
    }
    select.reset(session);
    session.close();
}

std::string FileDB::getFile(std::string endpt)
{
    Poco::Data::Session session("SQLite", dbfile);
    std::string extname = "";
    Statement select(session);
    std::string get_sql = "select extname from merge_templates where endpt=?";
    select << get_sql, use(endpt), into(extname);
    while (!select.done())
    {
        select.execute();
        break;
    }
    select.reset(session);
    session.close();
    return endpt + "." + extname;
}

std::string FileDB::getDocname(std::string endpt)
{
    Poco::Data::Session session("SQLite", dbfile);
    std::string extname = "";
    std::string docname = "";
    Statement select(session);
    std::string get_sql = "select docname, extname from merge_templates where endpt=?";
    select << get_sql, use(endpt), into(docname), into(extname);
    while (!select.done())
    {
        select.execute();
        break;
    }
    select.reset(session);
    session.close();
    return docname + "." + extname;
}
