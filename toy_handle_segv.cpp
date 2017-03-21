#include <iostream>

int sig_handler

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
    int cmd_type;
    std::cout<<"type 0 exit, type 1 print 1, type 2 generate segv"<<std::endl;
    for(;;) {
        std::cin >> cmd_type;
        if (cmd_type == 0) return 0;
        dispatcher(cmd_type);
    }
}