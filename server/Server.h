#ifndef CHATROOM_SERVER_H
#define CHATROOM_SERVER_H
#endif

#include <string>
 
#include "Common.h"
 
using namespace std;
 

// 服务端类，用来处理客户端请求
class Server {
 
public:
    // 无参数构造函数
    Server();
 
    // 初始化服务器端设置
    void Init();
 
    // 关闭服务
    void Close();
 
    // 启动服务端
    void Start();
    void dealCommand(char cmd);
        // 广播消息给所有客户端
    
    // 服务器端serverAddr信息
    struct sockaddr_in serverAddr;
    
    //创建监听的socket
    int listenFD;
 
    // epoll_create创建后的返回值
    int epFD;
    struct epoll_event ev,events[MAX_EVENT];
    // 客户端列表
    // std::set<int> connections;
};
void receiveFile(int,string);
void sendFile(int,string);
void describeFileOutput();
void * handle(void* );
void dealCommand(char cmd);
