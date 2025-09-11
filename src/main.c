// main.c
#include <stdio.h>
#include <pthread.h>

// 获取行数工具
int getFileLines()
{
    FILE *fp = fopen("input.txt", "r");
    if (!fp)
    {
        perror("Failed to open file");
        return 1;
    }

    int line_count = 0;
    char buffer[1024];

    while (fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        line_count++;
    }

    fclose(fp);

    printf("Total lines: %d\n", line_count);
    return line_count;
}

void readFileRange(int startLine, int endLine)
{
    FILE *file = fopen("input.txt", "r");
    if (!file)
    {
        perror("fopen");
        return;
    }

    char buffer[1024];
    int lineNum = 1;

    while (fgets(buffer, sizeof(buffer), file))
    {
        if (lineNum >= startLine && lineNum <= endLine)
        {
            printf("%s", buffer); // 这里可以改成存入数组
        }
        if (lineNum > endLine)
        {
            break;
        }
        lineNum++;
    }

    fclose(file);
}

int shared_data[4] = {0}; // 公共数组，初始都为 0
pthread_mutex_t lock;     // 互斥锁


int main(int argc, char *argv[])
{

    // 1.   处理原文件
    // 1.1  获取行数
    int fileLines = getFileLines();
    // 1.2  获取线程数
    printf("argc = %d\n", argc);

    if (argc < 3)
        return 1;

    int totalThreads = atoi(argv[2]);

    printf("n = %d\n", totalThreads);
    // 1.3  线程公共存储区确定自己读取的范围
    

    // 1.4  读取文件内容
    readFileRange(1, 4);

    // 2.   解析入参
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