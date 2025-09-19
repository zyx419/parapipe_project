#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

int stringTool(int argc, char *argv[])
{
    // save log to debug.log
    FILE *logfp = fopen("debug.log", "w");
    char *logvar[1024];
    if (logfp == NULL)
    {
        perror("fopen failed");
        return 1;
    }

    char buffer[1024];

    // save previous pipe read end (-1 means no previous pipe)
    int prevPipefd = -1;

    // TODO: "grep" change to variable
    // sprintf(logvar, "argv = %s , %s, %s, %s, %s, %s, %s\n", argv[0], argv[1], argv[2], argv[3], argv[4], argv[5], argv[6]);
    // log_debug(logfp, logvar);

    // anylize argv[5] to get command
    // char *test[] = {"grep abc -> grep 123 -> awk '{print $1}'", "->"};
    char *test[] = {"grep abc -> grep 123 -> grep 123 -> grep 123", "->"};
    // char *test[] = argv[0];

    // save result to command
    char *command[10];
    int commandCount = splitStringBy(2, test, command);
    sprintf(logvar, "command = %s\n", command[0]);
    log_debug(logfp, logvar);

    char *command2[10];

    // strcpy(command2[0], command[0]);

    for (int i = 0; i < commandCount; i++)
    {
        // create a pipe
        int pipefd[2];

        if (i < commandCount - 1)
        {
            pipe(pipefd);
        }

        sprintf(logvar, "command[%d] = %s\n", i, command[i]);
        log_debug(logfp, logvar);
        char *tempStr = command[i];

        test[0] = tempStr;
        test[1] = " ";

        sprintf(logvar, "test[0] = %s\n", test[0]);
        log_debug(logfp, logvar);

        sprintf(logvar, "test[1] = %s\n", test[1]);
        log_debug(logfp, logvar);

        splitStringBy(2, test, command2);

        // create a child process
        pid_t pid = fork();
        if (pid == 0)
        {
            // is not the first command
            if (prevPipefd != -1)
            {
                dup2(prevPipefd, STDIN_FILENO);
                // dup2(pipefd[1], STDOUT_FILENO);
                close(prevPipefd);
                // close(pipefd[1]);
            }
            // is not the last command
            if (i < commandCount - 1)
            {
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[1]);
            }

            char *const asd[] = {command2[0], command2[1], NULL};
            execvp(command2[0], asd);
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

            sprintf(logvar, "Child process finished\n");
            log_debug(logfp, logvar);

            // is not the first command
            if (prevPipefd != -1)
            {
                close(prevPipefd);
            }
            // is not the last command
            if (i < commandCount - 1)
            {
                close(pipefd[1]);
                prevPipefd = pipefd[0];
            }
        }
    }

    // free command "malloc(strlen(token) + 1)" in splitStringBy
    for (int i = 0; i < commandCount; i++)
    {
        free(command[i]);
    }

    // 等待所有子进程
    for (int i = 0; i < commandCount; i++)
    {
        wait(NULL);
    }

    return 0;
}
