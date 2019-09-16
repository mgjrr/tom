#include "T_client.h"
#include "log.h"

std::string SplitFilename (const std::string& str)
{
  std::size_t found = str.find_last_of("/");
  return str.substr(found+1);
}

T_client::T_client(std::string str)
{

    // init server address.
    client_name = str;
    log(SUC).out({"Creating client: ",client_name});

    server_addr.sin_family = PF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    client_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (client_fd < 0)
    {
        log(ERR).out({"Creating failed: "});
        exit(-1);
    }

    setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout_t, sizeof(timeout_t));
}

bool T_client::T_connect()
{
    
    if (connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        log(ERR).out({"Connect failed: "});
        exit(-1);
    }
}

bool T_client::T_close()
{

    close(client_fd);
    log(SUC).out({"Link ended. "});
}
bool T_client::login_check()
{
    std::string user_name,password;
    std::cout<<"user_name:";
    std::cin>>user_name;
    std::cout<<"password:";
    std::cin>>password;
    std::string qry = "L ";
    qry += user_name;
    qry += " ";
    //todo: use hash.
    qry += password;
    if(valid_check(qry))
    {
        std::cout<<"Log success."<<std::endl;
        return true;
    }
    else
    {
        std::cout<<"Log failed."<<std::endl;
        return false;
    }
}
bool T_client::T_start()
{

    T_connect();
    while(!login_check())
    {

    }
    log(SUC).out({"Connected."});
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
            // farewell();
            T_close();
            exit(0);
        default:
            break;
        }
    }
}
bool T_client::farewell()
{
    //todo : Add hash value.
    std::string bye_info = "";
    if (send(client_fd, bye_info.c_str(), bye_info.length(), 0) < 0)
    {
        log(ERR).out({"Diconnection info send Failed. "});
        return false;
    }
    return true;
}
bool T_client::valid_check(std::string query)
{
    char cmd_buf[CMD_SIZE];
    int length = -1;
    if (send(client_fd, query.c_str(), query.length(), 0) < 0)
    {
        log(ERR).out({"Command ",std::to_string(query[0]),"send Failed."});
        return false;
    }
    if ((length = recv(client_fd, cmd_buf, CMD_SIZE, 0)) != 1 || cmd_buf[0] != 'Y')
    {
        log(ERR).out({"Server rejected on ",std::to_string(query[0]),"."});
        return false;
    }
    return true;
}

bool T_client ::upload_file(const std::string & full_name)
{
    char send_buf[BUF_SIZE];
    int length = -1;
    FILE *fd;
    bool suc = true;
    std::string fileName = SplitFilename(full_name);

    std::string query = "U ";
    query += fileName;
    if (!valid_check(query))
        return false;

    if ((fd = fopen(full_name.c_str(), "r")) == NULL)
    {
        log(ERR).out({full_name," open failed. "});
        back_del(full_name);
        return false;
    }
    
    bzero(send_buf, sizeof(send_buf));
    while (!feof(fd))
    {
        int len = fread(send_buf, sizeof(char), sizeof(send_buf), fd);
        if (len != write(client_fd, send_buf, len))
        {
            log(ERR).out({"Write failed. "});
            suc = false;
            break;
        }
    }

    fclose(fd);
    log(SUC).out({"Trans suc.: "});

    return suc;
}

bool T_client ::download_file(std::string fileName)
{
    // std::cout << "fileName = " << fileName << std::endl;
    std::string fullPath = "/Users/ppp/tom/client/";
    fullPath += fileName;
    int length = -1;
    char recv_buf[BUF_SIZE];
    bool suc = true;

    std::string query = "D ";
    query += fileName;
    if (!valid_check(query))
        return false;

    FILE *fd;
    if ((fd = fopen(fullPath.c_str(), "w")) == NULL)
    {
        log(ERR).out({fileName," open failed. "});
        return false;
    }
    

    bzero(recv_buf, sizeof(recv_buf));
    while (length = recv(client_fd, recv_buf, BUF_SIZE, 0))
    {
        if (length == -1 && errno == EAGAIN)
        {
            printf("timeout.\n");
            log(WARN).out({"Timeout. "});
            break;
        }
        int write_length = fwrite(recv_buf, sizeof(char), length, fd);
        if (write_length < length)
        {
            log(ERR).out({"Write failed. "});
            suc = false;
            break;
        }
        bzero(recv_buf, sizeof(recv_buf));
    }

    fclose(fd);
    // printf("receive file %s success.\n", fullPath.c_str());
    return suc;
}

bool T_client ::browse_file()
{
    // std::cout << "Browse file on cloud." << std::endl;

    int length;
    std::string query = "B ";
    if (!valid_check(query))
        return false;
    char recv_buf[BUF_SIZE];

    bzero(recv_buf, sizeof(recv_buf));
    while (length = recv(client_fd, recv_buf, BUF_SIZE, 0))
    {
        if (length == -1 && errno == EAGAIN)
        {
            log(WARN).out({"Time out. "});
            break;
        }
        recv_buf[length] = '\0';
        printf("%s\n", recv_buf);
        bzero(recv_buf, sizeof(recv_buf));
    }
    // std::cout << "File browse end." << std::endl;
    return true;
}
bool T_client::back_del(std::string fileName)
{
    return true;
}