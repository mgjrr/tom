#ifndef T_SERVER_H
#define T_SERVER_H

#include "facility.h"

class T_server {
friend bool recv_file(int,std::string);
friend bool send_file(int,std::string);
friend bool browse_file();
public:
    T_server();
    ~T_server(){ }
    bool T_start();

private:
    struct sockaddr_in server_addr;
    bool T_close();
    bool dealCommand(char cmd);
};

void handle(int );
bool send_file(int,std::string);
bool recv_file(int,std::string);
bool browse_file();
bool describeFileOutput();
#endif
