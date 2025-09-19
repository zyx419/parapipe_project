// main.c
#include <stdio.h>
#include <unistd.h>

int testThread(int pipefd[])
{
    // int pipefd[2];

    // int id = *(int *)arg;
    // printf("子程序 %d 启动\n", id);

    char test[64];
    sprintf(test, "hello world from thread %d\n",2);
    // char *test[] = "hello world from thread \n";
    // pipe(pipefd);
    printf("子程序写入管道: %s", test);
    write(pipefd[1], test, strlen(test));
    // close(pipefd[1]);

    // printf("子程序 %d 完成\n", id);
    return pipefd[0];
}
