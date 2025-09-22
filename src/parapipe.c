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
pthread_mutex_t output_pipe_write_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t printf_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t string_tool_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct
{
    char *command;
    char *arguments;
    int input_pipe[2];
    int output_pipe[2];
} WorkerThreadArgs;

/*
    function: input reader thread that reads from stdin and writes to n pipes
    arg: input pipe array
    return: NULL
*/
void *input_reader(void *arg)
{
    int **input_pipes = (int **)arg;
    int pipe_count = input_pipes[0][0]; // first element stores pipe count
    char input_line[1024];
    int input_line_count = 0;
    int current_pipe = 0;

    while (fgets(input_line, sizeof(input_line), stdin) != NULL)
    {
        // write line to current pipe
        int current_pipe_fd = input_pipes[current_pipe + 1][1]; // +1 because first element is count
        pthread_mutex_lock(&input_pipe_write_mutex);
        ssize_t written = write(current_pipe_fd, input_line, strlen(input_line));
        pthread_mutex_unlock(&input_pipe_write_mutex);
        input_line_count++;

        if (written == -1)
        {
            perror("write to pipe failed");
            break;
        }

        // round-robin to next pipe
        current_pipe = (current_pipe + 1) % pipe_count;
    }

    // close all pipe write ends
    for (int i = 0; i < pipe_count; i++) {
        close(input_pipes[i + 1][1]); // +1 because first element is count
    }
    return NULL;
}

/*
    function: read results from output pipe and print them
    arg: output pipe read end
    return: NULL
*/
void *result_collector(void *arg)
{
    // read from output pipe
    int output_pipe_fd = *(int *)arg;

    char buf[4096];

    // set output pipe to non-blocking mode
    int flags = fcntl(output_pipe_fd, F_GETFL, 0);
    fcntl(output_pipe_fd, F_SETFL, flags | O_NONBLOCK);

    int timeout_count = 0;
    while (1)
    {
        int n = read(output_pipe_fd, buf, sizeof(buf) - 1);

        if (n > 0)
        {
            buf[n] = '\0';
            pthread_mutex_lock(&printf_mutex);
            printf("Result:\n%s\n", buf);
            pthread_mutex_unlock(&printf_mutex);

            timeout_count = 0;
        }
        else if (n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
        {
            // read every 100ms for faster response
            usleep(1000 * 100);
            timeout_count++;
            // 10 seconds timeout
            if (timeout_count > 100)
            {
                break;
            }
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
    function: worker thread that processes input data in batch
    arg: worker thread arguments
    return: NULL
*/
void *worker_thread(void *arg)
{
    WorkerThreadArgs *args = (WorkerThreadArgs *)arg;

    int input_pipe_fd = args->input_pipe[0];
    int output_pipe_fd = args->output_pipe[1];

    char buf[32768];
    int total_read = 0;
    ssize_t n;

    // read all data from pipe
    while ((n = read(input_pipe_fd, buf + total_read, sizeof(buf) - total_read - 1)) > 0) {
        total_read += n;
        
        // if buffer is full, stop reading
        if (total_read >= sizeof(buf) - 1) {
            break;
        }
    }
    buf[total_read] = '\0';

    if (total_read > 0) {
        // inputPara[0]: original command string (e.g. "grep abc -> grep 123")
        // inputPara[1]: string that needs be processed (e.g. "abc 123")
        char *command_args[] = {args->command, buf};

        // solve thread conflict problem
        pthread_mutex_lock(&string_tool_mutex);

        // call stringTool function to process the line, stringTool will put result to output_pipe_fd
        stringTool(command_args, output_pipe_fd);
        pthread_mutex_unlock(&string_tool_mutex);
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    // define output pipe for result collection
    int output_pipe[2];
    pipe(output_pipe);

    // create result collector thread
    pthread_t receiver_thread;
    if (pthread_create(&receiver_thread, NULL, result_collector, &output_pipe[0]) != 0)
    {
        perror("thread create failed");
        exit(1);
    }

    // default thread count 2
    int worker_thread_count = 2;
    char *command = NULL;
    
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-n") == 0 && i + 1 < argc)
        {
            worker_thread_count = atoi(argv[i + 1]);
            if (worker_thread_count <= 0)
            {
                worker_thread_count = 2;
            }
        }
        else if (strcmp(argv[i], "-c") == 0 && i + 1 < argc)
        {
            command = argv[i + 1];
        }
    }
    
    if (command == NULL)
    {
        fprintf(stderr, "Error: Command parameter (-c) is required\n");
        fprintf(stderr, "Usage: %s -c \"command\" [-n thread_count]\n", argv[0]);
        exit(1);
    }

    // create n input pipes for round-robin distribution
    int **input_pipes = malloc((worker_thread_count + 1) * sizeof(int*));
    input_pipes[0] = malloc(2 * sizeof(int));
    input_pipes[0][0] = worker_thread_count; // store pipe count in first element
    
    for (int i = 0; i < worker_thread_count; i++) {
        input_pipes[i + 1] = malloc(2 * sizeof(int));
        pipe(input_pipes[i + 1]);
    }

    // create input reader thread
    pthread_t input_reader_thread;
    if (pthread_create(&input_reader_thread, NULL, input_reader, input_pipes) != 0)
    {
        perror("input reader thread creation failed");
        exit(1);
    }

    pthread_t worker_threads[worker_thread_count];
    int thread_ids[worker_thread_count];

    // create worker thread arguments for each thread
    WorkerThreadArgs worker_args[worker_thread_count];
    for (int i = 0; i < worker_thread_count; i++) {
        worker_args[i].command = command;
        worker_args[i].output_pipe[0] = output_pipe[0];
        worker_args[i].output_pipe[1] = output_pipe[1];
        worker_args[i].input_pipe[0] = input_pipes[i + 1][0]; // read end
        worker_args[i].input_pipe[1] = input_pipes[i + 1][1]; // write end
    }

    // create worker threads
    for (int i = 0; i < worker_thread_count; i++)
    {
        thread_ids[i] = i + 1;
        if (pthread_create(&worker_threads[i], NULL, worker_thread, &worker_args[i]) != 0)
        {
            perror("worker thread creation failed");
            exit(1);
        }
    }

    // wait for input reader thread to finish
    pthread_join(input_reader_thread, NULL);

    // wait for all worker threads to finish
    for (int i = 0; i < worker_thread_count; i++)
    {
        pthread_join(worker_threads[i], NULL);
    }

    // close output pipe write end to inform receiver thread to exit
    close(output_pipe[1]);

    // wait for receiver thread to finish
    pthread_join(receiver_thread, NULL);

    // cleanup memory
    for (int i = 0; i <= worker_thread_count; i++) {
        free(input_pipes[i]);
    }
    free(input_pipes);

    return 0;
}
