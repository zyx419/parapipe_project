#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <semaphore.h>
#include "utils.h"

pthread_mutex_t input_pipe_read_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t input_pipe_write_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t string_tool_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct
{
    char *command;
    char *arguments;
    int input_pipe[2];
    int output_pipe[2];
} WorkerThreadArgs;

/*
    function: read results from output pipe and print them
    arg: output pipe read end
    return: NULL
*/
void *result_collector(void *arg)
{
    // read from output pipe
    int output_pipe_fd = *(int *)arg;

    char buf[256];

    // set output pipe to non-blocking mode
    int flags = fcntl(output_pipe_fd, F_GETFL, 0);
    fcntl(output_pipe_fd, F_SETFL, flags | O_NONBLOCK);

    while (1)
    {
        int n = read(output_pipe_fd, buf, sizeof(buf) - 1);

        if (n > 0)
        {
            buf[n] = '\0';
            printf("Result:\n%s", buf);
        }
        else if (n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
        {
            // read every 500ms
            usleep(1000 * 500);
        }
        else if (n == 0)
        {
            // EOF
            break;
        }
        else
        {
            perror("read");
            break;
        }
    }
    return NULL;
}

/*
    function: worker thread that processes input lines
    arg: worker thread arguments
    return: NULL
*/
void *worker_thread(void *arg)
{
    WorkerThreadArgs *args = (WorkerThreadArgs *)arg;

    int input_pipe_fd = args->input_pipe[0];
    char buf[256];
    int idx = 0;
    char c;

    // read one line from pipe
    while (idx < sizeof(buf) - 1)
    {
        // solve thread conflict problem
        pthread_mutex_lock(&input_pipe_read_mutex);
        ssize_t n = read(input_pipe_fd, &c, 1);
        pthread_mutex_unlock(&input_pipe_read_mutex);

        if (n <= 0)
            break;
        if (c == '\n')
            break;
        buf[idx++] = c;
    }
    buf[idx] = '\0';

    // inputPara[0]: original command string (e.g. "grep abc -> grep 123")
    // inputPara[1]: string that needs be processed (e.g. "abc 123")
    char *command_args[] = {args->command, buf};

    // fd: the pipe that receiver thread reads from
    int output_pipe_fd = args->output_pipe[1];

    // solve thread conflict problem
    pthread_mutex_lock(&string_tool_mutex);
    
    // call stringTool function to process the linel,stringTool will put result to output_pipe_fd
    stringTool(3, command_args, output_pipe_fd);
    pthread_mutex_unlock(&string_tool_mutex);

    return NULL;
}

int main(int argc, char *argv[])
{
    // define output pipe for result collection
    int output_pipe[2];
    pipe(output_pipe);
    // define input pipe for data distribution
    int input_pipe[2];
    pipe(input_pipe);

    // create result collector thread
    pthread_t result_collector_thread;
    if (pthread_create(&result_collector_thread, NULL, result_collector, &output_pipe[0]) != 0)
    {
        perror("thread create failed");
        exit(1);
    }

    char input_line[1024];
    int input_line_count = 0;
    while (fgets(input_line, sizeof(input_line), stdin) != NULL)
    {
        pthread_mutex_lock(&input_pipe_write_mutex);
        write(input_pipe[1], input_line, strlen(input_line));
        pthread_mutex_unlock(&input_pipe_write_mutex);
        input_line_count++;
    }

    close(input_pipe[1]);

    int worker_thread_count = -1;
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-n") == 0 && i + 1 < argc)
        {
            worker_thread_count = atoi(argv[i + 1]);
            break;
        }
    }
    if (worker_thread_count <= 0)
    {
        worker_thread_count = 2;
    }
    pthread_t worker_threads[worker_thread_count];
    int thread_ids[worker_thread_count];

    // create worker threads in batches
    for (int batch = 0; batch < input_line_count / worker_thread_count + 1; batch++)
    {
        for (int i = 0; i < worker_thread_count; i++)
        {
            // create worker thread arguments
            WorkerThreadArgs worker_args;
            worker_args.command = "grep abc -> grep 13";
            worker_args.output_pipe[0] = output_pipe[0];
            worker_args.output_pipe[1] = output_pipe[1];
            worker_args.input_pipe[0] = input_pipe[0];
            worker_args.input_pipe[1] = input_pipe[1];

            thread_ids[i] = i + 1;
            if (pthread_create(&worker_threads[i], NULL, worker_thread, &worker_args) != 0)
            {
                perror("worker thread creation failed");
                exit(1);
            }
        }

        // wait for current batch of worker threads to finish
        for (int i = 0; i < worker_thread_count; i++)
        {
            pthread_join(worker_threads[i], NULL);
        }
    }

    // loop end, close output pipe write end to inform receiver thread to exit
    close(output_pipe[1]);

    // wait for result collector thread to finish
    pthread_join(result_collector_thread, NULL);

    return 0;
}
