
#ifndef __MERGEODF_ACCESS_DB_H__
#define __MERGEODF_ACCESS_DB_H__
#include "config.h"
#include "Socket.hpp"
#include <string>
#include <memory>
/*
 * 可用來查詢呼叫次數
 */
class AccessDB
{
    public:
        AccessDB();
        ~AccessDB();

        void setDbPath();

        /// set endpoint
        void setApi(std::string Api)
        {
            api = Api;
        }

        void notice(std::weak_ptr<StreamSocket>,
                Poco::Net::HTTPResponse&,
                std::string);
        int getAccessTimes();
        void changeTable();
        void updateAccessTimes();

    private:
        std::string api;
        std::string dbfile;
};

#endif
