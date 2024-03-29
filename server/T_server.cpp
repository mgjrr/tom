#include <iostream>

#include "T_server.h"
#include "log.h"

// todo.
const std::string fileDes = "fileDes.txt";
int epoll_fd, listen_fd;
std::set<int> connections;
struct timeval timeout_t = {7, 0};
HiredisHelper redis;

void splitWithStrtok(const char* str, const char* delim, std::vector<std::string>& ret)
{
    char* strcopy = new char[strlen(str) + 1];
    strcpy(strcopy, str);
    char *word = strtok(strcopy, delim);
    ret.push_back(word);
    while (word = strtok(nullptr, delim))
        ret.push_back(word);
    delete[] strcopy;
}


// server.
bool T_server::user_check(const std::string & usn,const std::string & pwd)
{
    //todo.
    return redis_check_user(redis,{usn,pwd});
}

bool T_server::file_acc_check(int client_fd,const std::string & file_name)
{
    //todo.
    return true;
}
bool T_server::file_browse_check(int client_fd)
{
    //todo.
    return true;
}

bool T_server::valid_check(int client_fd,std::string &file_name)
{
    std::cout<<"begin check "<<client_fd<<std::endl;
    int length = 0;
    char cmd_buf[BUF_SIZE];
    // sleep(10); test for nianbao problem?
    if ((length = recv(client_fd, cmd_buf, CMD_SIZE, 0)) < 1)
    {
        log(WARN).out({"Client have quit."});
        connections.erase(client_fd);
        close(client_fd);
        return false;
    }
    char cmd = cmd_buf[0];
    cmd_buf[length] = '\0';
    std::cout<<"full cmd = "<<cmd_buf<<std::endl;
    std::vector<std::string > vec;
    splitWithStrtok(cmd_buf," ",vec);
    bool can = true;

    file_name = vec[1];

    file_name += vec[0];

    switch (cmd)
    {
    case 'L':
        can = user_check(vec[1],vec[2]);
        break;
    case 'U':
        can = file_acc_check(client_fd,vec[1]);
    case 'D':
        can = file_acc_check(client_fd,vec[1]);
    case 'B':
        can = file_browse_check(client_fd);
    default:
        break;
    }
    if(can)
    {
        if (send(client_fd, "Y", 1, 0) < 0)
        {
            log(ERR).out({std::string(__FILE__), std::string(__FUNCTION__), "Server admit on YES send Failed."});
            return can;
        }
        log(SUC).out({"check success."});
    }
    else
    {
        if (send(client_fd, "N", 1, 0) < 0)
        {
            log(ERR).out({std::string(__FILE__), std::string(__FUNCTION__), "Server admit on NO send Failed."});
            return can;
        }
        log(WARN).out({"check failed."});
    }
    return can;
}

void handle(task_info ti)
{

    // log(SUC).out({"Handle in the thread, id : ",std::to_string((int)std::this_thread::get_id())});

    //todo : valid check in server,not just say yes.

    

    std::cout << "Your cmd is " << ti.cmd << std::endl;
    switch (ti.cmd)
    {
    case 'U':
        recv_file(ti.client_fd, ti.fileName);
        break;
    case 'D':
        send_file(ti.client_fd, ti.fileName);
        break;
    case 'B':
        describeFileOutput();
        std::cout << "fileDes = " << fileDes << std::endl;
        send_file(ti.client_fd, fileDes);
        break;
    default:
        break;
    }

    // add this socket back.
    struct epoll_event ev;
    ev.data.fd = ti.client_fd;
    ev.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
    epoll_ctl(epoll_fd, EPOLL_CTL_MOD, ti.client_fd, &ev);

    log(SUC).out({"handle success"});
    return;
}
bool recv_file(int client_fd, std::string fileName)
{
    bool suc = true;

    char recv_buf[BUF_SIZE];
    std::cout << "fileName = " << fileName << std::endl;
    std::string fullPath = "/root/depository/";
    fullPath += fileName;
    std::cout << "fullPath  = " << fullPath << std::endl;

    FILE *fd;
    if ((fd = fopen(fullPath.c_str(), "w")) == NULL)
    {
        log(ERR).out({std::string(__FILE__), std::string(__FUNCTION__), "File open error."});
        suc = false;
        return suc;
    }

    bzero(recv_buf, sizeof(recv_buf));
    int length = 0;
    while (length = recv(client_fd, recv_buf, BUF_SIZE, 0))
    {
        if (length == -1 && errno == EAGAIN)
        {
            log(ERR).out({std::string(__FILE__), std::string(__FUNCTION__), "Time out"});
            break;
        }

        int write_length = fwrite(recv_buf, sizeof(char), length, fd);
        if (write_length < length)
        {
            log(ERR).out({std::string(__FILE__), std::string(__FUNCTION__), fullPath, " file write error."});
            suc = false;
            break;
        }
        bzero(recv_buf, sizeof(recv_buf));
    }

    fclose(fd);
    log(SUC).out({fullPath, " file receive success."});
    return suc;
}

bool send_file(int client_fd, std::string fileName)
{
    char send_buf[BUF_SIZE];
    bool suc = true;

    std::cout << "fileName = " << fileName << std::endl;
    std::string fullPath = "/root/depository/";
    fullPath += fileName;
    std::cout << "fullPath  = " << fullPath << std::endl;

    FILE *fd;
    if ((fd = fopen(fullPath.c_str(), "r")) == NULL)
    {
        log(ERR).out({std::string(__FILE__), std::string(__FUNCTION__), "File open error."});
        return false;
    }

    bzero(send_buf, sizeof(send_buf));
    while (!feof(fd))
    {
        int len = fread(send_buf, sizeof(char), sizeof(send_buf), fd);
        if (len != write(client_fd, send_buf, len))
        {
            printf("write error.\n");
            log(ERR).out({std::string(__FILE__), std::string(__FUNCTION__), fullPath, " file send error."});
            suc = false;
            break;
        }
    }

    fclose(fd);
    log(SUC).out({fullPath, " file send success."});
    return suc;
}

bool describeFileOutput()
{
    // std::cout << "at least here" << std::endl;
    log(SUC).out({"ls commmamd executed."});
    system("ls -l > /root/depository/fileDes.txt");
    return true;
}

bool T_server ::dealCommand(char cmd)
{
    if (cmd >= 'A' && cmd <= 'Z')
        cmd = cmd - 'A' + 'a';
    std::cout << "Your command is " << cmd << std::endl;
    if (cmd >= 'a' && cmd <= 'z')
    {
        if (cmd == 'b')
            system("ls -cl /root/depository");
        else if (cmd == 'q')
        {
            T_close();
            exit(0);
        }
        else
        {
            return true;
        }
    }
    else
    {
        std::cout << "Please input valid command" << std::endl;
    }
    return true;
}

T_server::T_server()
{

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    listen_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0)
    {
        log(ERR).out({std::string(__FILE__), std::string(__FUNCTION__), "listen socket create failed"});
        exit(-1);
    }
    if (bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        log(ERR).out({std::string(__FILE__), std::string(__FUNCTION__), "socket bind error", strerror(errno), std::to_string(errno)});
        exit(-1);
    }

    epoll_fd = epoll_create(MAX_EVENT);
    struct epoll_event ev;

    //add listener.
    ev.data.fd = listen_fd;
    ev.events = EPOLLIN | EPOLLET;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &ev);

    //listen to stdin.
    ev.data.fd = STDIN_FILENO;
    ev.events = EPOLLIN;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, STDIN_FILENO, &ev);

    log(SUC).out({"epoll create success"});
    int ret = listen(listen_fd, 317);
    if (ret < 0)
    {
        log(ERR).out({std::string(__FILE__), std::string(__FUNCTION__), "listen failed"});
        exit(-1);
    }

    log(SUC).out({"Start to listen: ", "inital ok"});
}

bool T_server::T_close()
{
    close(listen_fd);
    close(epoll_fd);
    for (auto i : connections)
    {
        // std::cout << "Have deleted " << i << std::endl;
        close(i);
    }
    log(SUC).out({"server end."});

    return true;
}

bool T_server::T_start(thread_pool &tp)
{
    std::string dontk = "dap1999";
    redis.Init("127.0.0.1",6379,dontk);
    struct epoll_event ev, events[MAX_EVENT];
    while (1)
    {
        int trigger_num = epoll_wait(epoll_fd, events, MAX_EVENT, 500);
        for (int i = 0; i < trigger_num; i++)
        {
            // check
            struct sockaddr_in sa;
            int len = sizeof(sa);
            getpeername(events[i].data.fd, (struct sockaddr *)&sa, (socklen_t *)&len);
            //accept new connection
            if (events[i].data.fd == listen_fd)
            {
                // to do.
                socklen_t addr_len = sizeof(listen_fd);

                int connect_fd = accept(listen_fd, NULL, &addr_len),tms = 0;
                std::string gjr = "";
                while(!valid_check(connect_fd,gjr))
                {
                    std::cout<<"log failed"<<std::endl;
                    tms++;
                    if(tms>=MAX_LOG_CHECK)
                    {
                        tms = PRES_CHECK_VAL;
                        break;
                    }
                }
                if(tms==PRES_CHECK_VAL) continue;
                connections.insert(connect_fd);
                ev.data.fd = connect_fd;
                ev.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, connect_fd, &ev);
                setsockopt(connect_fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout_t, sizeof(timeout_t));

                log(SUC).out({"A new connection founded,it's id : ", std::to_string(connect_fd)});
            }
            //from stdin.
            else if (events[i].data.fd == STDIN_FILENO)
            {
                std::cout << "Command received." << std::endl;
                char command_buf[BUF_SIZE];
                read(STDIN_FILENO, command_buf, sizeof(command_buf));
                dealCommand(command_buf[0]);
            }
            //read from a existed connection
            else
            {
                char cmd;
                int length = -1, tmp_fd = events[i].data.fd;
                std::string fileName;

                log(SUC).out({"A new event from connection founded. Client's IP is", inet_ntoa(sa.sin_addr), std::to_string(ntohs(sa.sin_port))});
                if (!valid_check(tmp_fd,fileName))
                    continue;
                cmd = fileName.back();
                fileName.pop_back();
                int tick = (int)(std::clock() * 1000 / CLOCKS_PER_SEC);
                task_info tmp_ti(cmd, tmp_fd, tick, fileName, NORMAL_PRI);
                tp.add_task(handle, tmp_ti);
            }
        }
    }
    return true;
}