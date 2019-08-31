#include <iostream>

#include "Client.h"

using namespace std;

struct timeval timeout = {33, 0};
int timeCheck()
{
    time_t now = time(0);
   
    char* dt = ctime(&now);
    cout << "当前时间" << dt << endl;
    return now;
}
// 客户端类成员函数

// 客户端类构造函数
Client::Client()
{

    // 初始化要连接的服务器地址和端口
    serverAddr.sin_family = PF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // 初始化socket
    sock = 0;

    // 初始化进程号
    pid = 0;

    // 客户端状态
    isClientwork = true;

    // epool fd
    epfd = 0;
}

// 连接服务器
void Client::Connect()
{
    cout << "Connect Server: " << SERVER_IP << " : " << SERVER_PORT << endl;

    // 创建socket
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("sock error");
        exit(-1);
    }

    // 连接服务端
    if (connect(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        perror("connect error");
        exit(-1);
    }
}

// 断开连接，清理并关闭文件描述符
void Client::Close()
{

    close(sock);
    cout << "your link end" << endl;
}

// 启动客户端
void Client::Start()
{

    // 连接服务器
    Connect();
    cout << "Connnect Success." << endl;
    while (1)
    {
        cout << "input command please. [u] for upload,[d] for download,[b] for browse files, [e] for terminate." << endl;
        string cmd;
        cin >> cmd;
        if (cmd[0] >= 'A' && cmd[0] <= 'Z')
            cmd[0] = cmd[0] - 'A' + 'a';
        switch (cmd[0])
        {
        case 'u':
            cout << "Input file's name to upload." << endl;
            cin >> cmd;
            uploadFile(cmd);
            break;
        case 'd':
            cout << "Input file's name to download." << endl;
            cin >> cmd;
            downloadFile(cmd);
            break;
        case 'b':
            browseFile();
            break;
        case 'q':
            Close();
            exit(0);
        default:
            break;
        }
    }
}

void Client ::uploadFile(string fileName)
{
    char CMD_BUF[CMD_SIZE];
    int length = -1;
    FILE * fd;
    string pres = "U";
    pres += fileName;
    if (send(sock, pres.c_str(), pres.length(), 0) < 0)
    {
        printf("Command [%s] send Failed.\n", "upload");
        return;
    }
    if ((length = recv(sock, CMD_BUF, CMD_SIZE, 0)) != 1 || CMD_BUF[0] != 'Y')
    {
        printf("Server  send back on [%s] error.\n", "upload");
        return;
    }
    if ((fd = fopen(fileName.c_str(), "r")) == NULL)
    {
        printf("File open error.\n");
        close(sock);
        exit(1);
    }
    int startTime = timeCheck();
    bzero(send_buf, sizeof(send_buf));
    while (!feof(fd))
    {
        int len = fread(send_buf, sizeof(char), sizeof(send_buf), fd);
        if (len != write(sock, send_buf, len))
        {
            printf("write error.\n");
            break;
        }
    }
    int endTime = timeCheck();
    cout<<"passed by "<<endTime-startTime<<endl;
    // 关闭文件
    fclose(fd);
    printf("File:%s Transfer Successful!\n", fileName.c_str());
}
void Client ::downloadFile(string fileName)
{
    cout << "fileName = " << fileName << endl;
    string fullPath = "/Users/ppp/tom/client/";
    fullPath += fileName;
    int length;
    string pres = "D";
    pres += fileName;
    if (send(sock, pres.c_str(), pres.length(), 0) < 0)
    {
        printf("Command [%s] send Failed.\n", "download");
        return;
    }
    if ((length = recv(sock, recv_buf, BUF_SIZE, 0)) != 1 || recv_buf[0] != 'Y')
    {
        printf("wrong? %d %c\n",length,recv_buf[0]);
        printf("Server send back on [%s] error.\n", "download");
        return;
    }

    FILE *fd;
    if ((fd = fopen(fullPath.c_str(), "w")) == NULL)
    {
        printf("File open error.\n");
        close(sock);
        exit(1);
    }

    int ret = setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout));
    int startTime = timeCheck();

    bzero(recv_buf, sizeof(recv_buf));
    while (length = recv(sock, recv_buf, BUF_SIZE, 0))
    {
        //cout << length << endl;
        if (length == -1 && errno == EAGAIN)
        {
            printf("timeout.\n");
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
    int endTime = timeCheck();
    cout<<"passed by "<<endTime-startTime<<endl;

    // 接收成功后，关闭文件，关闭socket
    fclose(fd);
    printf("receive file %s success.\n", fullPath.c_str());
}
void Client ::browseFile()
{
}