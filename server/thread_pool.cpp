#include "thread_pool.h"

auto tp_cmp = [](const thread_pool::task_pair & lhs,const thread_pool::task_pair & rhs){
        if(lhs.first.client_priority==rhs.first.client_priority)
            return lhs.first.acc_time > rhs.first.acc_time;
        return lhs.first.client_priority > rhs.first.client_priority;
    };
thread_pool::thread_pool(int x) : thread_n(x),is_on(false),task_queue(tp_cmp)
{

}

thread_pool::~thread_pool()
{
    {
        std::unique_lock<std::mutex> lck(mtx);
        is_on = false;
        cond_var.notify_all();
    }
    for(auto i: thread_list)
    {
        i->join();
        
        std::cout<<i<<" end "<<std::endl;
        delete i;
    }
    std::cout<<"all end"<<std::endl;
}

void thread_pool::thread_loop()
{
    // std::cout<<"create now"<<std::endl<<"I'm "<<std::this_thread::get_id()<<std::endl;
    while (is_on)
    {
        std::unique_lock<std::mutex> lck(mtx);
        while(is_on && task_queue.empty())
        {
            cond_var.wait(lck);
        }
        if(!is_on) break;
        // std::cout<<"I'm "<<std::this_thread::get_id()<<std::endl;
        task tmp_task = task_queue.top().second;
        task_queue.pop();
        std::cout<<"get it! "<<task_queue.size()<<std::endl;
        lck.unlock();
        tmp_task();
    }
    // std::cout<<is_on<<" time to go!"<<std::endl;
    // std::cout<<"I'm "<<std::this_thread::get_id()<<std::endl;
}
void thread_pool::start()
{
    is_on = true;
    for (int i = 0; i < thread_n; ++i)
    {
        std::cout<<i<<" created "<<std::endl;
        thread_list.push_back(new std::thread(std::bind(&thread_pool::thread_loop,this)));
    }
}
bool thread_pool::add_task(ori_task orit,task_info tki)
{
    //to do,for full.
    auto bdt = std::bind(orit,tki);
    
    std::unique_lock<std::mutex> lck(mtx);
    // std::cout<<"add suc"<<std::endl;
    task_queue.push(std::make_pair(tki,bdt));
    cond_var.notify_one();

    return true;
}