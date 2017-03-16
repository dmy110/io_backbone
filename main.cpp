#include <cstdio>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <string.h>
#include <string>
#include <iostream>

#define MAX_QUEUE_SIZE 1024 * 1024 * 64
struct transform_queue_t
{
    pthread_mutex_t mutex;
    uint32_t idx;
    unsigned char queue_data[0];
    void init()
    {
        idx = 0;
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        int err = pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
        if (err) perror("no support for shared mutex");
        pthread_mutex_init(&mutex, &attr);
        pthread_mutexattr_destroy(&attr);
    }

    void final()
    {
        pthread_mutex_destroy(&mutex);
    }

    int read(void* buffer, uint32_t buffer_len)
    {
        pthread_mutex_lock(&mutex);
        if (!idx) {
            pthread_mutex_unlock(&mutex);
            return 0;
        }

        if (buffer_len <= MAX_QUEUE_SIZE - idx) {
            pthread_mutex_unlock(&mutex);
            return -1;//bufferlen需要大于队列最大长度
        }
        uint32_t copy_len = idx;
        memcpy(buffer, queue_data, idx);
        idx = 0;
        pthread_mutex_unlock(&mutex);
        return copy_len;
    }   

    bool write(void* data, uint32_t data_len)
    {
        pthread_mutex_lock(&mutex);
        if (data_len + idx >= MAX_QUEUE_SIZE) {
            pthread_mutex_unlock(&mutex);
            return false;//空间不足，交给上层逻辑处理
        }

        memcpy(queue_data + idx, data, data_len);
        idx += data_len;
        pthread_mutex_unlock(&mutex);
        return true;
    }
};

transform_queue_t* p_2_c_queue;
transform_queue_t* c_2_p_queue;
unsigned char read_buffer[MAX_QUEUE_SIZE];
void parent();
void child();
int main()
{
    p_2_c_queue = (transform_queue_t*)mmap(nullptr, MAX_QUEUE_SIZE + sizeof(transform_queue_t), 
        PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANON, 0, 0);
    c_2_p_queue = (transform_queue_t*)mmap(nullptr, MAX_QUEUE_SIZE + sizeof(transform_queue_t), 
        PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANON, 0, 0);
    p_2_c_queue->init();
    c_2_p_queue->init();

    int pid = fork();
    if (pid) {
        parent();
        wait(nullptr);
    } else {
        child();
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
    unsigned char foo[64] = {0};
    int ret_size = recv(client, foo, sizeof(foo), 0);
    close(client);
    close(socketfd);

    p_2_c_queue->write(foo, ret_size);
}

void child()
{
    memset(read_buffer, 0, MAX_QUEUE_SIZE);
    while(true)
    {
        usleep(1000);
        int ret = p_2_c_queue->read(read_buffer, MAX_QUEUE_SIZE);
        if (ret == -1) return;//error handle
        if (ret) {
            std::string s((char*)read_buffer, ret);
            std::printf("%s\n", s.c_str());
            return;
        }
    }
}