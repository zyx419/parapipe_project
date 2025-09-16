// main.c
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

int main(int argc, char *argv[])
{
    char buffer[1024];
    // read stdin (only one line)
    // fgets(buffer, sizeof(buffer), stdin) != NULL;
    // printf("Read line: %s\n", buffer);

    // create a pipe
    int pipefd[2];
    // pipefd[0] read，pipefd[1] write
    pipe(pipefd);

    // TODO: "grep" change to variable
    printf("argv = %s , %s, %s, %s, %s, %s, %s\n", argv[0], argv[1], argv[2], argv[3], argv[4], argv[5], argv[6]);
    char *const asd[] = {"grep", "abc", NULL};

    // anylize argv[5] to get command
    char *test[] = {"grep abc -> grep 123 -> awk '{print $1}'", "->"};

    // save result to command
    char *command[10];
    int commandCount = splitStringBy(2, test, command);
    printf("command = %s\n", command[0]);

    char *command2[10];

    strcpy(command2[0], command[0]);

    for (int i = 0; i < commandCount; i++)
    {
        printf("command[%d] = %s\n", i, command[i]);
        char *tempStr = command[i];

        test[0] = tempStr;
        test[1] = " ";
        printf("test[0] = %s\n", test[0]);
        printf("test[1] = %s\n", test[1]);

        int commandCount = splitStringBy(2, test, command2);
    }

    // free command "malloc(strlen(token) + 1)" in splitStringBy
    for (int i = 0; i < commandCount; i++)
    {
        free(command[i]);
    }
    // free command "malloc(strlen(token) + 1)" in splitStringBy
    // for (int i = 0; i < commandCount; i++)
    // {
    //     free(command2[i]);
    // }

    // create a child process
    pid_t pid = fork();
    if (pid == 0)
    {
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
        execvp("grep", asd);
        perror("execvp failed");
        _exit(1);
    }
    else if (pid < 0)
    {
        perror("fork");
        return 1;
    }
    else
    {
        waitpid(pid, NULL, 0);
        printf("Child process finished\n");

        close(pipefd[1]);
        char buffer1[1024];
        int n;

        while ((n = read(pipefd[0], buffer1, sizeof(buffer1))) > 0)
        {
            printf("Read %d bytes from pipe\n", n);
            printf("buffer1 %s\n", buffer1);
            write(STDOUT_FILENO, buffer1, n);
            printf("Read %d bytes from pipe\n", n);
        }
        close(pipefd[0]);
    }
    return 0;
}
