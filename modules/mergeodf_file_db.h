#ifndef __MERGEODF_FILE_DB_H__
#define __MERGEODF_FILE_DB_H__
#include "config.h"
#include <Poco/Net/HTMLForm.h>

class FileDB
{
public:
    FileDB();
    virtual ~FileDB();

    virtual void setDbPath();

    std::string newkey();
    bool setFile(Poco::Net::HTMLForm &);
    void delFile(std::string);
    void updateFile(Poco::Net::HTMLForm &);
    std::string getFile(std::string);
    std::string getDocname(std::string);



private:
    std::string dbfile;
};

#endif
