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
#include <sys/epoll.h>
#define MAX_QUEUE_SIZE 1024 * 1024 * 64
#define MAX_EPOLL_EVENT 128 
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

//TODO(dmy) 非阻塞的server socket和阻塞的server socket的区别
//TODO(dmy) 设置发送接收缓存 或者交给os，这里只读取一下。
int get_server_socket(const std::string& addr, int port)
{
    int socketfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);

    inet_pton(AF_INET, addr.c_str(), &serveraddr.sin_addr.s_addr);
    int reuse = 1;
    setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    bind(socketfd, (const sockaddr*)&serveraddr, sizeof(serveraddr));
    int ret = listen(socketfd, 20);
    if (ret) {
        perror("listen");
        return 0;
    }
    return socketfd;
}

// int set_client_socket(int socket_fd)
// {

// }

void parent()
{
    void* read_buffer = malloc(MAX_QUEUE_SIZE);
    int server_fd = get_server_socket("127.0.0.1", 8890);

    struct epoll_event ev, events[MAX_EPOLL_EVENT];
    int epoll_fd = epoll_create(1);//args is ignored,just greater than 0
    ev.events = EPOLLIN;
    ev.data.fd = server_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev);

    int nfds = 0;
    for (;;) {
        nfds = epoll_wait(epoll_fd, events, MAX_EPOLL_EVENT, 1);//1ms的超时，做一些定时操作
        for (int n = 0; n < nfds; ++n) {
            if (events[n].data.fd == server_fd) {
                int client = accept(server_fd, nullptr, nullptr);
                fcntl(client, F_SETFL, O_NONBLOCK);
                ev.events = EPOLLIN;
                ev.data.fd = client;
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client, &ev);
                continue;
            }
            if (events[n].events == EPOLLIN) {
                //read
                int ret_size = TEMP_FAILURE_RETRY(recv(events[n].data.fd, read_buffer, MAX_QUEUE_SIZE, 0));
                if (ret_size == 0) {
                    //断开连接
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[n].data.fd, nullptr);
                    close(events[n].data.fd);
                    std::printf("close connect\n");
                }
                if (ret_size == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                    //
                    continue;
                }
                p_2_c_queue->write(read_buffer, ret_size);
            }
        }
        // unsigned char foo[64] = {0};
        // int ret_size = recv(client, foo, sizeof(foo), 0);
        // close(client);
    }
    close(server_fd);
    // p_2_c_queue->write(foo, ret_size);
    free(read_buffer);
}

// unsigned char read_buffer_queue[MAX_QUEUE_SIZE];
void child()
{
    void *read_buffer = malloc(MAX_QUEUE_SIZE);
    memset(read_buffer, 0, MAX_QUEUE_SIZE);
    while(true)
    {
        //TODO(dmy)修改为 fixed timestamp
        usleep(1000);
        int ret = p_2_c_queue->read(read_buffer, MAX_QUEUE_SIZE);
        if (ret == -1) break;//error handle
        if (ret) {
            std::string s((char*)read_buffer, ret);
            std::printf("%s\n", s.c_str());
            // break;
        }
    }
    free(read_buffer);
}