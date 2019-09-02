#include "T_client.h"

int timeCheck()
{
    time_t now = time(0);

    char *dt = ctime(&now);
    std::cout << "Current time :" << dt << std::endl;
    return now;
}

T_client::T_client(std::string str)
{

    // init server address.
    client_name = str;
    std::cout << "Creating client: " << client_name << std::endl;

    server_addr.sin_family = PF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    client_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (client_fd < 0)
    {
        perror("client_fd error");
        exit(-1);
    }

    setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout_t, sizeof(timeout_t));
}

bool T_client::T_connect()
{

    if (connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Connect error");
        exit(-1);
    }
}

bool T_client::T_close()
{

    close(client_fd);
    std::cout << "Your link end." << std::endl;
}

bool T_client::T_start()
{

    // 连接服务器
    T_connect();
    std::cout << "Connnect Success." << std::endl;
    while (1)
    {
        std::cout << "input command please. [u] for upload,[d] for download,[b] for browse files, [q] for terminate." << std::endl;
        std::string cmd;
        std::cin >> cmd;
        if (cmd[0] >= 'A' && cmd[0] <= 'Z')
            cmd[0] = cmd[0] - 'A' + 'a';
        switch (cmd[0])
        {
        case 'u':
            std::cout << "Input file's name to upload." << std::endl;
            std::cin >> cmd;
            upload_file(cmd);
            break;
        case 'd':
            std::cout << "Input file's name to download." << std::endl;
            std::cin >> cmd;
            download_file(cmd);
            break;
        case 'b':
            browse_file();
            break;
        case 'q':
            T_close();
            exit(0);
        default:
            break;
        }
    }
}

bool T_client::validCheck(std::string query)
{
    char cmd_buf[CMD_SIZE];
    int length = -1;
    if (send(client_fd, query.c_str(), query.length(), 0) < 0)
    {
        printf("Command [%c] send Failed.\n", query[0]);
        return false;
    }
    if ((length = recv(client_fd, cmd_buf, CMD_SIZE, 0)) != 1 || cmd_buf[0] != 'Y')
    {
        printf("Server send back command on [%c] error.\n", query[0]);
        return false;
    }
    return true;
}

bool T_client ::upload_file(std::string fileName)
{
    char send_buf[BUF_SIZE];
    int length = -1;
    FILE *fd;
    bool suc = true;

    std::string query = "U";
    query += fileName;
    if (!validCheck(query))
        return false;

    if ((fd = fopen(fileName.c_str(), "r")) == NULL)
    {
        printf("File open error.\n");
        back_del(fileName);
        return false;
    }
    int startTime = timeCheck();
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
    int endTime = timeCheck();
    std::cout << "passed by " << endTime - startTime << std::endl;

    fclose(fd);
    printf("File:%s Transfer Successful!\n", fileName.c_str());
    return suc;
}

bool T_client ::download_file(std::string fileName)
{
    std::cout << "fileName = " << fileName << std::endl;
    std::string fullPath = "/Users/ppp/tom/client/";
    fullPath += fileName;
    int length = -1;
    char recv_buf[BUF_SIZE];
    bool suc = true;

    std::string query = "D";
    query += fileName;
    if (!validCheck(query))
        return false;

    FILE *fd;
    if ((fd = fopen(fullPath.c_str(), "w")) == NULL)
    {
        printf("File open error.\n");
        return false;
    }

    int startTime = timeCheck();

    bzero(recv_buf, sizeof(recv_buf));
    while (length = recv(client_fd, recv_buf, BUF_SIZE, 0))
    {
        if (length == -1 && errno == EAGAIN)
        {
            printf("timeout.\n");
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
    int endTime = timeCheck();
    std::cout << "passed by " << endTime - startTime << std::endl;

    fclose(fd);
    printf("receive file %s success.\n", fullPath.c_str());
    return suc;
}

bool T_client ::browse_file()
{
    std::cout << "Browse file on cloud." << std::endl;

    int length;
    std::string query = "B";
    if (!validCheck(query))
        return false;
    char recv_buf[BUF_SIZE];

    bzero(recv_buf, sizeof(recv_buf));
    while (length = recv(client_fd, recv_buf, BUF_SIZE, 0))
    {
        if (length == -1 && errno == EAGAIN)
        {
            printf("timeout.\n");
            break;
        }
        recv_buf[length] = '\0';
        printf("%s\n", recv_buf);
        bzero(recv_buf, sizeof(recv_buf));
    }
    std::cout << "File browse end." << std::endl;
    return true;
}
bool T_client::back_del(std::string fileName)
{
    return true;
}