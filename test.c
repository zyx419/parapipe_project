/* simple_exec.c */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }
    if (pid == 0) {
        // 子进程：用 execvp 执行 grep
        char *argv[] = {"grep", "abc", "largefile.txt", NULL};
        execvp(argv[0], argv);
        // execvp 成功后不会返回，失败则返回 -1
        perror("execvp");
        _exit(127);
    } else {
        // 父进程：等待子进程结束
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            printf("child exited with %d\n", WEXITSTATUS(status));
        } else {
            printf("child terminated abnormally\n");
        }
    }
    return 0;
}
