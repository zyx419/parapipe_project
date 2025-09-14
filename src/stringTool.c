// main.c
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
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

    // create a child process
    pid_t pid = fork();
    if (pid == 0)
    {
        // TODO: "grep" change to variable
        printf("argv = %s , %s, %s, %s, %s, %s, %s\n", argv[0], argv[1], argv[2], argv[3], argv[4], argv[5], argv[6]);
        char *const asd[] = {"grep","abc", NULL};

        // anylize argv[5] to get command
        char *test[] = {"grep abc -> grep 123 -> awk '{print $1}'", "->"};
        
        char *command[] = {"0"};
        int commandCount = splitStringBy(2, test, command);
        printf("command = %s\n", command[1]);
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
