#include "T_server.h"
#include "log.h"
#include "thread_pool.h"

int main(int argc, char *argv[]) 
{

    thread_pool * tp = new thread_pool(5);
    tp->start();
    
    T_server tom;
    tom.T_start(*tp);
    tom.T_close();

    delete tp;
    return 0;
}