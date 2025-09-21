#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#include <fcntl.h>

// External mutex declarations
extern pthread_mutex_t output_pipe_write_mutex;

// todo: change to variable
#define MAX_COMMANDS 10


// trim spaces for char*
char *trim(char *str)
{
    char *end;

    // delete space at beginning
    while (isspace((unsigned char)*str))
        str++;

    if (*str == 0)
        return str;

    // delete space at last
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end))
        end--;

    // add '\0' (for "strncpy" function)
    *(end + 1) = '\0';

    return str;
}

/*
    function: split string by separator, save to resultCommands[]
    argv[0]: original command string (e.g. "grep abc -> grep 123 -> awk '{print $1}'")
    argv[1]: separator string (e.g. "->")
    resultCommands[]: array to save split commands (e.g. "grep abc")
    return: command count
*/
int splitStringBy(int argc, char *argv[], char *resultCommands[])
{
    // oriCommandStr (e.g. "grep abc -> grep 123 -> awk '{print $1}'");
    char oriCommandStr[1024];
    // copy argv[0] to oriCommandStr
    strncpy(oriCommandStr, argv[0], sizeof(oriCommandStr) - 1);
    // ensure null-termination
    oriCommandStr[sizeof(oriCommandStr) - 1] = '\0';


    // define commands array to save split result(e.g."grep abc")
    char *commands[MAX_COMMANDS];

    // define count to save command count
    int count = 0;

    char *seperater = argv[1];
    // split by seperater
    char *token = strtok(oriCommandStr, seperater);
    while (token != NULL && count < MAX_COMMANDS)
    {
        // trim space
        token = trim(token);
        // solve malloc issue
        resultCommands[count] = malloc(strlen(token) + 1);
        strcpy(resultCommands[count], token);
        count++;
        // NULL means get next token
        token = strtok(NULL, seperater);
    }

    return count;
}

/*
    function: execute piped commands
    argv[0]: original command string (e.g. "grep abc -> grep 123")
    argv[1]: string that needs be processed (e.g. "abc 123")
    argv[2]: the pipe that receiver thread reads from（output pipe write end）
    return: 0
*/
int stringTool(int argc, char *argv[], int fd)
{
    // printf("DEBUG: stringTool start processing: %s\n", argv[1]);
    
    // save previous pipe read end (-1 means no previous pipe)
    int prevPipefd = -1;

    // save original stdin
    int original_stdin = dup(STDIN_FILENO);

    // put pipe data to stdin for first loop
    int initPipefd[2];
    pipe(initPipefd);
    dup2(initPipefd[0], STDIN_FILENO);
    // write data to pipe
    write(initPipefd[1], argv[1], strlen(argv[1]));
    close(initPipefd[1]);

    // argv[0]: original command string (e.g. "grep abc -> grep 123 -> grep 123 -> grep 123")
    char *inputPara[] = {argv[0], "->"};

    // save result to command
    char *splitedCommand[MAX_COMMANDS];

    // execute splitStringBy function get command count and splitedCommand[]
    int commandCount = splitStringBy(2, inputPara, splitedCommand);

    // Linux command breakdown (e.g. "grep" , "abc")
    char *tempCommand[2];

    char buffer[1024];

    // create child process in a loop to execute each command(e.g. "grep abc")
    for (int i = 0; i < commandCount; i++)
    {
        // create a pipe
        int pipefd[2];

        // is not the last command
        if (i < commandCount - 1)
        {
            pipe(pipefd);
        }

        inputPara[0] = splitedCommand[i];
        inputPara[1] = " ";

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
                // write stdout to fd (the pipe that receiver thread reads from)
                dup2(fd, STDOUT_FILENO);
            }

            // make commandArgs for execvp (e.g. {"grep", "abc", NULL})
            char *const commandArgs[] = {tempCommand[0], tempCommand[1], NULL};
            // execute command
            execvp(tempCommand[0], commandArgs);

            // if execvp returns, there must be an error
            perror("execvp failed");
            _exit(1);
        }
        // is parent process
        else
        {
            // wait for child process to finish
            waitpid(pid, NULL, 0);

            // is not the last command
            if (i < commandCount - 1)
            {
                // close current pipe write end
                close(pipefd[1]);
                // save current pipe read end for next command
                prevPipefd = pipefd[0];
            }
        }
    }

    for (int i = 0; i < commandCount; i++)
    {
        free(splitedCommand[i]);
    }
    
    // restore original stdin
    dup2(original_stdin, STDIN_FILENO);
    close(original_stdin);
    
    return 0;
}