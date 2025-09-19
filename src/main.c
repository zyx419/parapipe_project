#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "stringTool.h"

void *receiver(void *arg)
{
    int fd = *(int *)arg; // 从 void* 恢复成 int

    char buf[256];

    // 设置非阻塞，避免阻塞线程
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);

    while (1)
    {
        int n = read(fd, buf, sizeof(buf) - 1);
        if (n > 0)
        {
            buf[n] = '\0';
            printf("Read: %s", buf);
        }
        else if (n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
        {
            // 暂时没有数据
            usleep(1000); // 可加延时避免忙等
        }
        else if (n == 0)
        {
            // EOF，管道关闭
            break;
        }
        else
        {
            perror("read");
            break;
        }
    }
    return NULL;
}

// void *sub_task(int *arg)
void *sub_task(void *arg)
{
    // int *pipefd = (int *)arg;
    char *test[] = {"grep abc -> grep 123 -> grep 123 -> grep 123", "->"};
    int i = stringTool(2, test);
    printf("子程序返回的管道读端i: %d\n", i);
    // printf("子程序返回的管道读端: %d\n", pipefd[0]);

    return NULL;
}
// void *sub_task(int *arg)
// {
//     int *pipefd = (int *)arg;
//     int i = testThread(pipefd);
//     printf("子程序返回的管道读端i: %d\n", i);
//     printf("子程序返回的管道读端: %d\n", pipefd[0]);

//     return NULL;
// }

int main()
{
    int pipefd[2];
    pipe(pipefd);
    // 创建接收线程

    // pthread_t idsa;
    // if (pthread_create(&idsa, NULL, receiver, &pipefd[0]) != 0)
    // {
    //     perror("线程创建失败");
    //     exit(1);
    // }

    const int THREAD_COUNT = 3;
    pthread_t threads[THREAD_COUNT];
    int ids[THREAD_COUNT];

    // 主程序调用子程序时，多开线程执行
    for (int i = 0; i < THREAD_COUNT; i++)
    {
        ids[i] = i + 1;
        if (pthread_create(&threads[i], NULL, sub_task, pipefd) != 0)
        {
            perror("线程创建失败");
            exit(1);
        }
    }

    // 等待所有子线程结束
    for (int i = 0; i < THREAD_COUNT; i++)
    {
        pthread_join(threads[i], NULL);
    }
    // pthread_join(idsa, NULL);

    printf("主程序结束\n");
    return 0;
}
