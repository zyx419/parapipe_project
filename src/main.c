// main.c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int main(int argc, char *argv[])
{

    // 1.   read stdin line by line -> put to threads
    // 1.1  get totalThreads
    printf("argc = %d\n", argc);

    if (argc < 3)
        return 1;

    int totalThreads = atoi(argv[2]);

    printf("n = %d\n", totalThreads);

    char buffer[1024];

    // read stdin by line
    while (fgets(buffer, sizeof(buffer), stdin) != NULL)
    {
        printf("Read line: %s\n", buffer);

        // todo:make threads
    }
    // 2.   解析入参
    char *inputChar = argv[4];
    printf("inputChar = %s\n", inputChar);

    


    // 2.1  分割入参
    // 2.2  保存到队列中

    // 3.   处理数据

    // 3.1  根据入参执行对应操作
    // 3.2  获取输出结果

    // 4.   输出结果
    // 4.1  输出到对应的接受数据进程

    return 0;
}

// void *thread_func(void *arg)
// {
//     // shared_data += 1; // 修改共享数据
//     // printf("Thread %ld, shared_data=%d\n", (long)arg, shared_data);
//     // return NULL;
// }