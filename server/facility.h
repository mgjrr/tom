#ifndef TOM_FACILITY_H
#define TOM_FACILITY_H
 
#include <iostream>
#include <list>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctime>
#include <fstream>
#include <set>
#include <thread>
#include <vector>
#include <mutex>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SERVER_IP "49.235.218.112"

#define SERVER_PORT 8888

#define CMD_SIZE 100

#define BUF_SIZE 0xFFFF

#define MAX_EVENT 20

#endif