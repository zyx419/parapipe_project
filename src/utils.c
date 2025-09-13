// main.c
#include <stdio.h>
#include <stdlib.h>

#

int main(int argc, char *argv[])
{

    // 执行一个命令，例如 ls -l
    int ret = system("findstr a1bc");
    if (ret == -1)
    {
        perror("system");
    }
    return 0;
}
