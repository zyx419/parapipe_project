// main.c
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    char buffer[1024];
    // read stdin (only one line)
    fgets(buffer, sizeof(buffer), stdin) != NULL;
    printf("Read line: %s\n", buffer);

    // create a pipe
    int pipefd[2];
    // pipefd[0] read，pipefd[1] write
    pipe(pipefd);

    // create a child process
    pid_t pid = fork();

    // 'child process' put 'stdout' to 'pipe write'
    dup2(pipefd[1], STDOUT_FILENO);

    close(pipefd[0]);
    close(pipefd[1]);

    //TODO: "grep" change to variable
    execvp("grep", argv);

    return 0;
}
