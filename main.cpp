#include <cstdio>
#include <unistd.h>
//test multi process mutex
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/socket.h>
// #include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <string.h>
#include <string>
#include <iostream>
// #include <sys/types.h>
pthread_mutex_t* mutex;
void* shm_buffer;

struct io_data_t
{
    uint32_t data_len;
    unsigned char* data[0];
};

void parent();
void child();
int main()
{
    mutex = (pthread_mutex_t*)mmap(nullptr, sizeof(pthread_mutex_t), PROT_READ|PROT_WRITE,
        MAP_SHARED|MAP_ANON, 0, 0);
    shm_buffer = mmap(nullptr, 1024, PROT_READ|PROT_WRITE,
        MAP_SHARED|MAP_ANON, 0, 0);
    memset(shm_buffer, 0, 1024);

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    int err = pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    if (err) perror("");
    pthread_mutex_init(mutex, &attr);
    pthread_mutexattr_destroy(&attr);
    int pid = fork();
    if (pid) {
        //父进程锁住，等十秒
        // pthread_mutex_lock(mutex);
        // std::printf("呵呵思密达\n");
        // sleep(10);
        // pthread_mutex_unlock(mutex);
        // wait(nullptr);
        // sleep(1);//
        parent();
    } else {
        child();
        // sleep(1);//
        // pthread_mutex_lock(mutex); 
        // std::printf("wtf!!\n");
        // pthread_mutex_unlock(mutex);
    }
    return 0;
}

void parent()
{
    int socketfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(8890);

    char addr[] = "127.0.0.1";
    inet_pton(AF_INET, addr, &serveraddr.sin_addr.s_addr);
    int reuse = 1;
    setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    bind(socketfd, (const sockaddr*)&serveraddr, sizeof(serveraddr));
    int ret = listen(socketfd, 20);
    if (ret) {
        perror("listen");
        return;
    }

    int client = accept(socketfd, nullptr, nullptr);
    char foo[64] = {0};
    recv(client, foo, sizeof(foo), 0);
    // printf("%s\n", foo);
    close(client);
    close(socketfd);

    pthread_mutex_lock(mutex);
    io_data_t* trans_data = (io_data_t*)shm_buffer;
    trans_data->data_len = strlen(foo);
    memcpy(trans_data->data, foo, trans_data->data_len);
    pthread_mutex_unlock(mutex);
}

void child()
{
    while(true)
    {
        usleep(1000);
        if (pthread_mutex_trylock(mutex) == 0) {
            io_data_t* trans_data = (io_data_t*)shm_buffer;
            if (trans_data->data_len) {
                std::string s((char*)trans_data->data, trans_data->data_len);
                std::printf("%s\n", s.c_str());
                pthread_mutex_unlock(mutex);
                break;
            }
            pthread_mutex_unlock(mutex);
        }
    }
}