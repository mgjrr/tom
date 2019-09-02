#ifndef T_CLIENT_H
#define T_CLIENT_H

#include "facility.h"


class T_client {

public:
    T_client(std::string str = "tom_client");
    ~T_client(){ }
    bool T_start();
private:
    std::string client_name;
    int client_fd;
    struct sockaddr_in server_addr;
    struct timeval timeout_t = {7, 0};
    bool T_connect();
    bool T_close();
    bool upload_file(std::string);
    bool download_file(std::string);
    bool browse_file();
    bool validCheck(std::string);
    bool back_del(std::string);
};

#endif
