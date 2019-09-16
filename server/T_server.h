#ifndef T_SERVER_H
#define T_SERVER_H

#include "facility.h"
#include "thread_pool.h"
#include "redis.h"

class T_server {
friend bool recv_file(int,std::string);
friend bool send_file(int,std::string);
friend bool browse_file();
public:
    T_server();
    ~T_server(){ }
    bool T_start(thread_pool &);
    bool T_close();

private:
    struct sockaddr_in server_addr;
    std::map<int,std::string> fd2usn_map; 
    bool dealCommand(char cmd);
    bool valid_check(int ,std::string &);
    bool user_check(const std::string &,const std::string & );
    bool file_acc_check(int ,const std::string &);
    bool file_browse_check(int );
};

void handle(task_info);
bool send_file(int,std::string);
bool recv_file(int,std::string);
bool browse_file();
bool describeFileOutput();
#endif
