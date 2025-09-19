#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <semaphore.h>
#include "stringTool.h"

pthread_mutex_t printMutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct
{
    char *cmd;
    char *args;
    int mainPipefd[2];
    int receiverPipefd[2];
    // int fd2[2];
} ThreadArg;

/*
    function: read from pipe non-blocking
    arg: pipe read end
    return: NULL
*/
void *receiver(void *arg)
{

    int fd = *(int *)arg;

    char buf[256];

    // set fd to non-blocking mode
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);

    while (1)
    {

        printf("receiver Thread %lu \n", (unsigned long)pthread_self());

        int n = read(fd, buf, sizeof(buf) - 1);
        if (n > 0)
        {
            buf[n] = '\0';
            printf("receiver Thread %lu:Read: %s\n", (unsigned long)pthread_self(), buf);
        }
        else if (n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
        {
            // read every 1ms
            usleep(1000 * 1000 * 5);
        }
        else if (n == 0)
        {
            // EOF
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

/*
    function: open thread to call stringTool
    arg: pipe read end
    return: NULL
*/
void *sub_task(void *arg)
{
    ThreadArg *data = (ThreadArg *)arg;

    int fd1 = data->mainPipefd[0];
    char buf[256];
    int idx = 0;
    char c;

    while (idx < sizeof(buf) - 1)
    {
        ssize_t n = read(fd1, &c, 1); // 一次读一个字节
        if (n <= 0)
            break; // 管道关闭或错误
        if (c == '\n')
            break; // 遇到换行就结束，但不加入缓冲区
        buf[idx++] = c;
    }
    buf[idx] = '\0'; // 字符串结束符
    if (idx > 0)
    {
        pthread_mutex_lock(&printMutex);
        printf("work Thread %lu: Read first line: %s\n", (unsigned long)pthread_self(), buf);
        fflush(stdout);
        pthread_mutex_unlock(&printMutex);
    }

    // inputPara[0]: original command string (e.g. "grep abc -> grep 123")
    // inputPara[1]: string that needs be processed (e.g. "abc 123")
    char *inputPara[] = {data->cmd, buf};
    // printf("Thread command: %s \n", data->cmd);
    int fd = data->receiverPipefd[1];
    // int fd2 = data->mainPipefd[0];
    // fd: the pipe that receiver thread reads from
    stringTool(3, inputPara, fd);

    return NULL;
}

int main()
{
    // define a pipe that receiver thread reads from
    int receiverPipefd[2];
    pipe(receiverPipefd);
    // define a pipe that main thread writes to
    int mainPipefd[2];
    pipe(mainPipefd);

    // create receiver thread
    pthread_t receiverThread;
    if (pthread_create(&receiverThread, NULL, receiver, &receiverPipefd[0]) != 0)
    {
        perror("thread create failed");
        exit(1);
    }

    char buffer[1024];
    int count = 0;
    while (fgets(buffer, sizeof(buffer), stdin) != NULL)
    {
        write(mainPipefd[1], buffer, strlen(buffer));
        count++;
    }
    close(mainPipefd[1]);

    const int THREAD_COUNT = 3;
    pthread_t threads[THREAD_COUNT];
    int ids[THREAD_COUNT];

    // create sub threads
    for (int j = 0; j < count / THREAD_COUNT + 1; j++)
    {
        printf("Main thread: Creating batch %d of sub threads\n", j + 1);
        for (int i = 0; i < THREAD_COUNT; i++)
        {
            // argv[0]: original command string (e.g. "grep abc -> grep 123")
            // argv[1]: string read from STDIN that needs be processed (e.g. "abc 123")
            // argv[2]: the pipe that receiver thread reads from

            // create thread argument
            ThreadArg inputPara;
            inputPara.cmd = "grep abc -> grep 132";
            inputPara.receiverPipefd[0] = receiverPipefd[0];
            inputPara.receiverPipefd[1] = receiverPipefd[1];
            inputPara.mainPipefd[0] = mainPipefd[0];
            inputPara.mainPipefd[1] = mainPipefd[1];

            ids[i] = i + 1;
            if (pthread_create(&threads[i], NULL, sub_task, &inputPara) != 0)
            {
                perror("thread create failed");
                exit(1);
            }
        }

        // wait for sub threads to finish
        for (int i = 0; i < THREAD_COUNT; i++)
        {
            pthread_join(threads[i], NULL);
        }
    }
    pthread_join(receiverThread, NULL);

    // printf("sub threads finish\n");
    return 0;
}
