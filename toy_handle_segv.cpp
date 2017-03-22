#include <iostream>
#include <signal.h>
#include <ucontext.h>
#include <errno.h>
#include <string.h>
ucontext_t main_context;
char stack_buffer[64 * 1024];

void sig_handler(int signum)
{
    std::printf("sig catch:%d, %s\n", signum, strerror(errno));
    std::printf("try to restore context\n");
    std::printf("restore success, continue type your cmd\n");
    setcontext(&main_context);
}

int dispatcher(int type)
{
    if (type == 1) {
        std::cout<<1<<std::endl;
    } else if (type == 2) {
        char* foo = nullptr;
        *foo = 'a';
    }
}

int main()
{
    struct sigaction act;
    act.sa_handler = sig_handler;
    sigset_t sigmask;
    sigemptyset(&sigmask);
    sigaddset(&sigmask, SIGSEGV);
    sigaction(SIGSEGV, &act, nullptr);
    
    int cmd_type;
    std::cout<<"type 0 exit, type 1 print 1, type 2 generate segv"<<std::endl;
    for(;;) {
        getcontext(&main_context);
        main_context.uc_stack.ss_sp = stack_buffer;
        main_context.uc_stack.ss_size = sizeof(stack_buffer);
        std::cin >> cmd_type;
        if (cmd_type == 0) return 0;
        dispatcher(cmd_type);
    }
}