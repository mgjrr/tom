#ifndef LOG_H
#define LOG_H

#define DEBUG 1

#include <mutex>
#include <iostream>
#include <fstream>


typedef enum log_rank
{
    SUC,
    WARN,
    ERR
} log_rank_t;

class log
{
    friend void init_log_stream(std::initializer_list<std::string>);

private:
    log_rank_t m_lgt;

public:
    static std::ofstream stm_arr[3];
    static std::mutex mtx_arr[3];
    // log(){}
    log(log_rank_t lgt) : m_lgt(lgt) {}
    std::ostream &ret_stm();
    void out(std::initializer_list<std::string>);
};

void init_log_stream(std::initializer_list<std::string>);

#endif