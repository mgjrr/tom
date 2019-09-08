#include <iostream>

#include "T_server.h"


using namespace std;

const string fileDes = "fileDes.txt";
int epoll_fd, listen_fd;
std::set<int> connections;
struct timeval timeout_t = {7, 0};

void handle(task_info ti)
{

    std::thread::id id = std::this_thread::get_id();
    std::cout << "In the new thread, id : " << id << std::endl;

    //todo : valid check in server,not just say yes.

    if (send(ti.client_fd, "Y", 1, 0) < 0)
    {
        printf("Server admit [%s] send Failed./n", "yes");
        return;
    }

    cout << "Your cmd is " << ti.cmd << endl;
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
        cout << "fileDes = " << fileDes << endl;
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

    cout << "handle success" << endl;
    return;
}
bool recv_file(int client_fd, string fileName)
{
    bool suc = true;

    char recv_buf[BUF_SIZE];
    cout << "fileName = " << fileName << endl;
    string fullPath = "/root/depository/";
    fullPath += fileName;
    cout << "fullPath  = " << fullPath << endl;

    FILE *fd;
    if ((fd = fopen(fullPath.c_str(), "w")) == NULL)
    {
        printf("File open error.\n");
        suc = false;
        return suc;
    }

    bzero(recv_buf, sizeof(recv_buf));
    int length = 0;
    while (length = recv(client_fd, recv_buf, BUF_SIZE, 0))
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
            suc = false;
            break;
        }
        bzero(recv_buf, sizeof(recv_buf));
    }

    fclose(fd);
    printf("receive file %s success.\n", fullPath.c_str());
    return suc;
}

bool send_file(int client_fd, string fileName)
{
    char send_buf[BUF_SIZE];
    bool suc = true;

    cout << "fileName = " << fileName << endl;
    string fullPath = "/root/depository/";
    fullPath += fileName;
    cout << "fullPath  = " << fullPath << endl;

    FILE *fd;
    if ((fd = fopen(fullPath.c_str(), "r")) == NULL)
    {
        printf("File open error.\n");
        return false;
    }

    bzero(send_buf, sizeof(send_buf));
    while (!feof(fd))
    {
        int len = fread(send_buf, sizeof(char), sizeof(send_buf), fd);
        if (len != write(client_fd, send_buf, len))
        {
            printf("write error.\n");
            suc = false;
            break;
        }
    }

    fclose(fd);
    printf("send file %s success.\n", fullPath.c_str());
    return suc;
}

bool describeFileOutput()
{
    cout << "at least here" << endl;
    system("ls -l > /root/depository/fileDes.txt");
    return true;
}

bool T_server ::dealCommand(char cmd)
{
    if (cmd >= 'A' && cmd <= 'Z')
        cmd = cmd - 'A' + 'a';
    cout << "Your command is " << cmd << endl;
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
        cout << "Please input valid command" << endl;
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
        perror("listen_fd");
        exit(-1);
    }
    if (bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("bind socket error: %s(errno: %d)\n", strerror(errno), errno);
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

    cout << "epoll create success" << endl;

    int ret = listen(listen_fd, 317);
    if (ret < 0)
    {
        perror("listen error");
        exit(-1);
    }

    cout << "Start to listen: " << SERVER_IP << endl;
    cout << "inital ok" << endl;
}

bool T_server::T_close()
{
    close(listen_fd);
    close(epoll_fd);
    for (auto i : connections)
    {
        cout << "Have deleted " << i << endl;
        close(i);
    }
    cout << "server end." << endl;

    return true;
}

bool T_server::T_start(thread_pool & tp)
{
    struct epoll_event ev, events[MAX_EVENT];
    while (1)
    {
        int trigger_num = epoll_wait(epoll_fd, events, MAX_EVENT, 500);
        // vector<thread> vec_thd;
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

                int connect_fd = accept(listen_fd, NULL, &addr_len);
                connections.insert(connect_fd);
                ev.data.fd = connect_fd;
                ev.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, connect_fd, &ev);
                setsockopt(connect_fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout_t, sizeof(timeout_t));

                cout << "A new connection founded,it's id : " << connect_fd << endl;
            }
            //from stdin.
            else if (events[i].data.fd == STDIN_FILENO)
            {
                cout << "Command received.\n";
                char command_buf[BUF_SIZE];
                read(STDIN_FILENO, command_buf, sizeof(command_buf));
                dealCommand(command_buf[0]);
            }
            //read from a existed connection
            else
            {
                char cmd_buf[CMD_SIZE];
                int length = -1, tmp_fd = events[i].data.fd;
                string fileName;

                cout << "A new event from connection founded.  ";
                printf("Client's IP is %s:%d\n", inet_ntoa(sa.sin_addr), ntohs(sa.sin_port));

                if ((length = recv(tmp_fd, cmd_buf, CMD_SIZE, 0)) < 1)
                {
                    printf("Client have quit.\n");
                    connections.erase(tmp_fd);
                    close(tmp_fd);
                    continue;
                }
                for (int i = length - 1; i > -1; --i)
                    if (cmd_buf[i] == '/' || i == 0)
                    {
                        for (int j = i + 1; j < length; ++j)
                            fileName.push_back(cmd_buf[j]);
                        break;
                    }
                int tick=(int)(std::clock()*1000/ CLOCKS_PER_SEC);
                task_info tmp_ti(cmd_buf[0],tmp_fd,tick,fileName,NORMAL_PRI);
                tp.add_task(handle,tmp_ti);
                // thread fresh_t(handle, tmp_fd, cmd_buf[0], fileName);

                printf("New task added.\n");

                // without this detach(),thread will dump,because the new thread here is a local variable, when leaving else segment,it will be release.
                // join will wait for it.
                // fresh_t.detach();
            }
        }
    }
    return true;
}