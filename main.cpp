#include <cstdio>
#include <unistd.h>
//test multi process mutex
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
pthread_mutex_t mutex;
int main()
{
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    int err = pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    if (err) perror("");
    pthread_mutex_init(&mutex, &attr);
    pthread_mutexattr_destroy(&attr);
    int pid = fork();
    if (pid) {
        //父进程锁住，等十秒
        pthread_mutex_lock(&mutex);
        std::printf("呵呵思密达\n");
        sleep(10);
        pthread_mutex_unlock(&mutex);
        wait(nullptr);
        // sleep(1);//
    } else {
        sleep(1);//
        pthread_mutex_lock(&mutex); 
        std::printf("wtf!!\n");
        pthread_mutex_unlock(&mutex);
    }
    return 0;
}