#ifndef THREAD_POOL_H
#define THREAD_POOL_H
#include <queue>
#include <iostream>
#include <vector>
#include <list>
#include <thread>
#include <mutex>
#include <condition_variable>

enum task_priority
{
    LOW_PRI,
    NORMAL_PRI,
    URGE_PRI
};

struct task_info
{
    char cmd;
    int client_fd;
    int acc_time;
    std::string fileName;
    task_priority client_priority;
    task_info(char a,int b,int c,std::string d,task_priority e):
    cmd(a),client_fd(b),acc_time(c),fileName(d),client_priority(e){ }
};

class thread_pool
{
public:
    using ori_task = std::function<void(task_info)>;
    using task = std::function<void()>;
    using task_pair = std::pair<task_info,task>;
    thread_pool(int x);
    ~thread_pool();
    void start();
    bool add_task(ori_task,task_info);

private:
    const int thread_n;
    bool is_on;
    std::list<std::thread *> thread_list;
    std::priority_queue<task_pair,std::vector<task_pair>,std::function<bool(const task_pair&,const task_pair&)> > task_queue;
    std::mutex mtx;
    std::condition_variable cond_var;

    thread_pool(const thread_pool &);
    thread_pool & operator=(const thread_pool &);
    void thread_loop();


};





#endif