#include "T_client.h"
#include "log.h"

int main(int argc, char *argv[]) {
    init_log_stream({"/Users/ppp/tom/client/Suc_stream.txt","/Users/ppp/tom/client/Warn_stream.txt","/Users/ppp/tom/client/Err_stream.txt"});
    T_client tom;
    tom.T_start();
    return 0;
}