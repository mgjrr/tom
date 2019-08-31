#include "Server.h"
 
// 服务端主函数
// 创建服务端对象后启动服务端
int main(int argc, char *argv[]) {
    // if(fork())
    // {
    //     string cmd;
    //     while(cin>>cmd)
    //     {
    //         if(cmd=="B"||cmd=="b")
    //             system("ls");
    //         if(cmd=="END")
    //             break;
    //     }
    //     exit(0);
    // }
    Server server;
    server.Start();
    return 0;
}