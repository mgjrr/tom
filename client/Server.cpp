//Server.cpp
#include <iostream>

#include "Server.h"

using namespace std;

// 服务端类成员函数
struct timeval timeout = {3, 0}; //3s
int global_epFD,global_listenFD;
void* handle(void * threadIn)
{
    int clientFD = *(int*)threadIn;
    char cmd_buf[CMD_SIZE];
    int length = -1;
    if ((length = recv(clientFD, cmd_buf, CMD_SIZE, 0)) < 1)
    {
        // printf("length = %d, %c \n",length,cmd_buf[0]);
        printf("Client command error,maybe client have quit.\n");
        close(clientFD);
        return NULL;
    }
    if (send(clientFD, "Y", 1, 0) < 0)
    {
        printf("Server admit [%s] send Failed./n", "yes");
        return NULL;
    }
    string fileName;
    for (int i = length - 1; i > -1; --i)
    {
        if (cmd_buf[i] == '/'||i==0)
        {
            for (int j = i + 1; j < length; ++j)
            {
                fileName.push_back(cmd_buf[j]);
            }
            break;
        }
    }
    switch (cmd_buf[0])
    {
    case 'U':
        receiveFile(clientFD, fileName);
        break;
    case 'D':
        sendFile(clientFD, fileName);
        break;
    case 'B':
        describeFile();
        break;
    default:
        break;
    }
    // add this socket back.
    struct epoll_event ev;
    ev.data.fd = clientFD;
    ev.events = (( EPOLLIN | EPOLLET )| EPOLLONESHOT | EPOLLHUP);
    epoll_ctl(global_epFD, EPOLL_CTL_MOD, clientFD, &ev);

    cout<<"handle success"<<endl;
    return NULL;

}
void receiveFile(int clientFD, string fileName)
{
    char recv_buf[BUF_SIZE];
    cout << "fileName = " << fileName << endl;

    string fullPath = "/root/server/";
    fullPath += fileName;
    cout << "fullPath  = " << fullPath << endl;
    FILE *fd;

    if ((fd = fopen(fullPath.c_str(), "w")) == NULL)
    {
        printf("File open error.\n");
        close(clientFD);
        exit(1);
    }

    int ret = setsockopt(clientFD, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout));
    bzero(recv_buf, sizeof(recv_buf));
    int length = 0;
    while (length = recv(clientFD, recv_buf, BUF_SIZE, 0))
    {
        //cout << length << endl;
        if (length == -1 && errno == EAGAIN)
        {
            printf("timeout\n");
            break;
        }

        int write_length = fwrite(recv_buf, sizeof(char), length, fd);
        if (write_length < length)
        {
            printf("error at write.\n");
            break;
        }
        bzero(recv_buf, sizeof(recv_buf));
        //cout << write_length << endl;
    }
    // 接收成功后，关闭文件，关闭socket
    fclose(fd);
    printf("receive file %s success.\n", fullPath.c_str());

}
void sendFile(int clientFD, string fileName)
{
    char send_buf[BUF_SIZE];

    cout << "fileName = " << fileName << endl;

    string fullPath = "/root/server/";
    fullPath += fileName;
    cout << "fullPath  = " << fullPath << endl;
    FILE *fd;

    if ((fd = fopen(fullPath.c_str(), "r")) == NULL)
    {
        printf("File open error.\n");
        close(clientFD);
        exit(1);
    }

    bzero(send_buf, sizeof(send_buf));
    while (!feof(fd))
    {
        int len = fread(send_buf, sizeof(char), sizeof(send_buf), fd);
        if (len != write(clientFD, send_buf, len))
        {
            printf("write error.\n");
            break;
        }
    }
    // 接收成功后，关闭文件，关闭socket
    fclose(fd);
    printf("send file %s success.\n", fullPath.c_str());
    return;
}

void describeFile()
{

    
    return;
}

void dealCommand(char cmd)
{
    if(cmd>='A'&&cmd<='Z') cmd = cmd-'A'+'a';
    cout<<"Your command is "<<cmd<<endl;
    if(cmd>='a'&&cmd<='z')
    {
        if(cmd=='b')
            system("ls -l");
        if(cmd=='q')
        {
            close(global_listenFD);
        }
    }
    else
    {
        cout<<"Please input valid command"<<endl;
    }
}
// 服务端类构造函数
Server::Server()
{

    // 初始化服务器地址和端口
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    // 初始化socket
    listenFD = 0;

    // epool fd
    epFD = 0;
    cout << "inital ok" << endl;
}
// 初始化服务端并启动监听
void Server::Init()
{
    cout << "Init Server..." << endl;

    //创建监听socket
    listenFD = socket(PF_INET, SOCK_STREAM, 0);
    global_listenFD = listenFD;
    if (listenFD < 0)
    {
        perror("listenFD");
        exit(-1);
    }

    //绑定地址
    if (bind(listenFD, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        printf("bind socket error: %s(errno: %d)\n", strerror(errno), errno);
        exit(-1);
    }

    epFD = epoll_create(MAX_EVENT);
    global_epFD = epFD;
    
    //add listener.
    ev.data.fd = listenFD;
    ev.events = EPOLLIN | EPOLLET;
    epoll_ctl(epFD, EPOLL_CTL_ADD, listenFD, &ev);

    //listen to stdin.
    ev.data.fd = STDIN_FILENO;
    ev.events = EPOLLIN ;
    epoll_ctl(epFD, EPOLL_CTL_ADD, STDIN_FILENO, &ev);

    cout << "epoll create success" << endl;

    int ret = listen(listenFD, 5);
    if (ret < 0)
    {
        perror("listen error");
        exit(-1);
    }

    cout << "Start to listen: " << SERVER_IP << endl;
}

// 关闭服务，清理并关闭文件描述符
void Server::Close()
{

    //关闭socket
    close(listenFD);

    //关闭epoll监听
    close(epFD);
    cout << "server end." << endl;
}


// 启动服务端
void Server::Start()
{

    // 初始化服务端
    Init();

    while (1)
    {
        int trigger_num = epoll_wait(epFD, events, MAX_EVENT, 500);
        for (int i = 0; i < trigger_num; i++)
        {
            // check
            struct sockaddr_in sa;
            int len = sizeof(sa);
            getpeername(events[i].data.fd, (struct sockaddr *)&sa, (socklen_t *)&len);
            //accept new connection
            if (events[i].data.fd == listenFD)
            {
                // to do.
                socklen_t addr_len = sizeof(listenFD);
                int connect_fd = accept(listenFD, NULL, &addr_len);

                connections.insert(connect_fd);
                ev.data.fd = connect_fd;
                ev.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
                epoll_ctl(epFD, EPOLL_CTL_ADD, connect_fd, &ev);
                cout << "a new connection!  "<<endl;
            }
            //from stdin.
            else if(events[i].data.fd == STDIN_FILENO)
            {
                cout<<"Command received.\n";
                char command_buf[BUF_SIZE];
                read(STDIN_FILENO, command_buf, sizeof(command_buf));
                dealCommand(command_buf[0]);
            }
            //read from a existed connection
            else
            {
                
                pthread_t newThreadId;
                cout<<"A new event from connection founded.  ";
                printf("对方IP %s:%d\n", inet_ntoa(sa.sin_addr), ntohs(sa.sin_port));

                int newThread = pthread_create(&newThreadId,NULL,handle,(void *)(&events[i].data.fd));
                printf("now create thread %d.\n",(int)newThreadId);

                // handle(events[i].data.fd);
            }
        }
    }
    Close();
}