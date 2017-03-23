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
#include "log.h"
using namespace Seamless;
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

struct transform_cmd_t
{
    uint8_t cmd_type = 0;
    uint32_t cmd_size = 0;
};

//TODO(dmy) 不应该使用sock-fd，使用自己生成的唯一标志id
struct transform_cmd_socket_data_t : transform_cmd_t
{
    int sock_fd;
    uint32_t data_len;
    unsigned char* data[0];
};


transform_queue_t* p_2_c_queue;
transform_queue_t* c_2_p_queue;
int pipefd[2];

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
    pipe2(pipefd, O_NONBLOCK);

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
    LogUtils::init();
    log("test if log is ok:%s", "parent");
    void* pipe_discard_buffer = malloc(1024);
    void* read_buffer = malloc(MAX_QUEUE_SIZE * 2);
    void* write_buffer = malloc(MAX_QUEUE_SIZE * 2);

    int server_fd = get_server_socket("127.0.0.1", 8890);

    struct epoll_event ev, events[MAX_EPOLL_EVENT];
    int epoll_fd = epoll_create(1);//args is ignored,just greater than 0
    ev.events = EPOLLIN;
    ev.data.fd = server_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev);

    ev.events = EPOLLIN;
    ev.data.fd = pipefd[0];
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, pipefd[0], &ev);

    int nfds = 0;
    for (;;) {
        nfds = epoll_wait(epoll_fd, events, MAX_EPOLL_EVENT, -1);//可以设置1ms的超时，做一些定时操作
        for (int n = 0; n < nfds; ++n) {
            if (events[n].data.fd == server_fd) {
                int client = accept(server_fd, nullptr, nullptr);
                fcntl(client, F_SETFL, O_NONBLOCK);//TODO(dmy) 是否设置nodelay？
                ev.events = EPOLLIN;
                ev.data.fd = client;
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client, &ev);
                continue;
            }
            if (events[n].events == EPOLLIN && events[n].data.fd != pipefd[0]) {
                //read
                transform_cmd_socket_data_t* recv_data = (transform_cmd_socket_data_t*)read_buffer;
                recv_data->sock_fd = events[n].data.fd;

                int ret_size = TEMP_FAILURE_RETRY(recv(events[n].data.fd, recv_data->data, MAX_QUEUE_SIZE, 0));
                if (ret_size == 0) {
                    //断开连接
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[n].data.fd, nullptr);
                    close(events[n].data.fd);
                    std::printf("close connect\n");
                    continue;
                }
                if (ret_size == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                    //
                    continue;
                }
                recv_data->cmd_size = ret_size + sizeof(transform_cmd_socket_data_t);
                recv_data->data_len = ret_size;
                p_2_c_queue->write(read_buffer, recv_data->cmd_size);
            }
            if (events[n].events == EPOLLOUT) {
                continue;
            }
            if (events[n].data.fd == pipefd[0]) {
                //wake up by pipe
            }
        }
        //read and discard from the pipe
        read(pipefd[0], pipe_discard_buffer, 1024);
        //read from child
        int ret = c_2_p_queue->read(write_buffer, MAX_QUEUE_SIZE);
        //handle the cmd from child
        if (ret == -1) continue;//error happened
        if (ret < sizeof(transform_cmd_t)) {
            continue;//should not happened
        }
        while (ret) {
            transform_cmd_t* cmd = (transform_cmd_t*)write_buffer;
            if (cmd->cmd_type == 0) {
                //socket data
                //just echo
                transform_cmd_socket_data_t* send_data = (transform_cmd_socket_data_t*)write_buffer;
                int ret = send(send_data->sock_fd, send_data->data, send_data->data_len, 0);//TODO(dmy) 非阻塞io，需要处理一下
                if (ret == -1) {
                    perror("send");
                }
            }

            ret = ret >= cmd->cmd_size ? ret - cmd->cmd_size : 0;
        }
    }
    close(server_fd);
    free(read_buffer);
    free(pipe_discard_buffer);
    free(write_buffer);
}

// unsigned char read_buffer_queue[MAX_QUEUE_SIZE];
void child()
{
    LogUtils::init();
    log("test if log is ok,%s", "child");
    void *read_buffer = malloc(MAX_QUEUE_SIZE * 2);
    void *write_buffer = malloc(MAX_QUEUE_SIZE * 2);
    memset(read_buffer, 0, MAX_QUEUE_SIZE);

    char pipe_discard = 'a';
    while(true)
    {
        //TODO(dmy)修改为 fixed timestamp
        usleep(1000);
        int ret = p_2_c_queue->read(read_buffer, MAX_QUEUE_SIZE);
        if (ret == -1) break;//error handle
        if (ret) {
            if (ret < sizeof(transform_cmd_t)) {
                break;//should not happened
            }

            while (ret) {
                transform_cmd_t* cmd = (transform_cmd_t*)read_buffer;
                if (cmd->cmd_type == 0) {
                    //socket data
                    //just echo
                    transform_cmd_socket_data_t* recv_data = (transform_cmd_socket_data_t*)read_buffer;
                    transform_cmd_socket_data_t* send_data = (transform_cmd_socket_data_t*)write_buffer;
                    memcpy(send_data, recv_data, recv_data->cmd_size);
                    c_2_p_queue->write(send_data, send_data->cmd_size);
                    write(pipefd[1], &pipe_discard, sizeof(pipe_discard));
                    std::string s((char*)recv_data->data, recv_data->data_len);
                    std::printf("recv_dat:%s\n", s.c_str());
                }

                ret = ret >= cmd->cmd_size ? ret - cmd->cmd_size : 0;
            }
            // break;
        }
    }
    free(read_buffer);
    free(write_buffer);
}