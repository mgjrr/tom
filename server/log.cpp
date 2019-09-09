#include "log.h"

std::mutex log::mtx_arr[3];
std::ofstream log::stm_arr[3];

void init_log_stream(std::initializer_list<std::string> il)
{
    int cnt = 0;
    for (auto i : il)
    {
        log::stm_arr[cnt].open(i);
        cnt++;
    }
}

std::ostream &log::ret_stm()
{
    // return std::cout;
    // return log::stm_arr[m_lgt];
    return DEBUG ? (std::cout) : (log::stm_arr[m_lgt]);
}

void log::out(std::initializer_list<std::string> il)
{
    log::mtx_arr[m_lgt].lock();
    time_t now = time(0);
    std::string to_out = "At " + std::string(ctime(&now));
    for (auto is : il)
    {
        to_out += " ";
        to_out += is;
    }
    std::ostream &osr = ret_stm();
    osr << to_out << std::endl;
    log::mtx_arr[m_lgt].unlock();
}