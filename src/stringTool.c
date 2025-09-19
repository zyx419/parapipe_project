#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "utils.h"
#include "stringTool.h"

extern pthread_mutex_t printMutex;
// todo: change to variable
#define MAX_COMMANDS 10

/*
    function: execute piped commands
    argv[0]: original command string (e.g. "grep abc -> grep 123")
    argv[1]: string that needs be processed (e.g. "abc 123")
    argv[2]: the pipe that receiver thread reads from
    return: the pipe that receiver thread reads from
*/
int stringTool(int argc, char *argv[], int fd)
{

    // save log to debug.log
    FILE *logfp = fopen("debug.log", "w");
    char *logvar[1024];
    if (logfp == NULL)
    {
        perror("fopen failed");
        return 1;
    }

    // save previous pipe read end (-1 means no previous pipe)
    int prevPipefd = -1;
    // initialize stdin with the string that needs be processed
    int initPipefd[2];
    pipe(initPipefd);
    dup2(initPipefd[0], STDIN_FILENO);
    write(initPipefd[1], argv[1], strlen(argv[1]));
    close(initPipefd[1]);

    // argv[0]: original command string (e.g. "grep abc -> grep 123 -> grep 123 -> grep 123")
    char *inputPara[] = {argv[0], "->"};
    // printf("Thread %lu:Original string: [%s]\n", (unsigned long)pthread_self(), inputPara[0]);

    // save result to command
    char *splitedCommand[MAX_COMMANDS];

    // execute splitStringBy function get command count and splitedCommand[]
    int commandCount = splitStringBy(2, inputPara, splitedCommand);

    // Linux command breakdown (e.g. "grep" , "abc")
    char *tempCommand[2];

    // strcpy(command2[0], command[0]);
    char buffer[1024];

    // create child process in a loop to execute each command(e.g. "grep abc")
    for (int i = 0; i < commandCount; i++)
    {
        // create a pipe
        int pipefd[2];
        int lastPipefd[2];
        pipe(lastPipefd);

        // is not the last command
        if (i < commandCount - 1)
        {
            pipe(pipefd);
        }
        // is the last command (no need to create pipe)

        sprintf(logvar, "command[%d] = %s\n", i, splitedCommand[i]);
        log_debug(logfp, logvar);

        inputPara[0] = splitedCommand[i];
        inputPara[1] = " ";

        sprintf(logvar, "inputPara[0] = %s inputPara[1] = %s\n", inputPara[0], inputPara[1]);
        log_debug(logfp, logvar);

        // split command by space (e.g. "grep abc" -> "grep", "abc")
        splitStringBy(2, inputPara, tempCommand);

        // create a child process
        pid_t pid = fork();

        // is child process
        if (pid == 0)
        {
            // is not the first command
            if (prevPipefd != -1)
            {
                // read from previous pipe(instead of stdin)
                dup2(prevPipefd, STDIN_FILENO);
                close(prevPipefd);
            }
            // is the first command (read from stdin, do nothing)

            // is not the last command
            if (i < commandCount - 1)
            {
                // write to current pipe(instead of stdout)
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[1]);
            }
            // is the last command (write to stdout, do nothing)
            else
            {
                //     dup2(lastPipefd[1], STDOUT_FILENO);
                dup2(fd, STDOUT_FILENO);
                //     close(lastPipefd[0]);
                //     close(lastPipefd[1]);
                close(fd);
            }

            // make commandArgs for execvp (e.g. {"grep", "abc", NULL})
            char *const commandArgs[] = {tempCommand[0], tempCommand[1], NULL};
            // execute command
            execvp(tempCommand[0], commandArgs);

            // if execvp returns, there must be an error
            perror("execvp failed");
            _exit(1);
        }
        else if (pid < 0)
        {
            // if fork() returns -1, there was an error
            perror("fork");
            return 1;
        }
        // is parent process
        else
        {
            // wait for child process to finish
            waitpid(pid, NULL, 0);

            sprintf(logvar, "Child process finished\n");
            log_debug(logfp, logvar);

            // is not the first command
            if (prevPipefd != -1)
            {
                // close previous pipe read end
                close(prevPipefd);
            }
            // is the first command (no previous pipe to close)

            // is not the last command
            if (i < commandCount - 1)
            {
                // close current pipe write end
                close(pipefd[1]);
                // save current pipe read end for next command
                prevPipefd = pipefd[0];
            }
            // is the last command
            else
            {
                // save current pipe read end for receiver thread
                // prevPipefd = pipefd[0];
                // write to receiver thread pipe
                // char readBuffer[1024];
                // int n = read(lastPipefd[0], readBuffer, sizeof(readBuffer) - 1);
                // if (n > 0)
                // {
                //     readBuffer[n] = '\0'; // null-terminate the string
                //     write(fd, readBuffer, n);
                // }
                // close(lastPipefd[0]);
            }
        }
    }

    for (int i = 0; i < commandCount; i++)
    {
        free(splitedCommand[i]);
    }
    // fclose(logfp);
    // printf("Thread %lu:Finished\n", (unsigned long)pthread_self());
    return 0;
}
