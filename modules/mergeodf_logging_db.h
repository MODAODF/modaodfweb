
#ifndef __MERGEODF_LOGGING_DB_H__
#define __MERGEODF_LOGGING_DB_H__
#include "config.h"
#include "Socket.hpp"
#include <string>
#include <memory>
/*
 * 紀錄 log 用:
 * 將 log 紀錄到 sqlite, 可用來查詢呼叫次數
 */
class LogDB
{
    public:
        LogDB();
        ~LogDB();
        void addRecord(int status, int to_pdf,
                std::string source_ip,
                std::string file_name,
                std::string file_ext,
                std::string cost,
                std::string timestamp);
        std::string getRecord(std::string);

    private:
        void setDbPath();
        std::string dbfile;
};

#endif
