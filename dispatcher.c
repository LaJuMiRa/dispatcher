#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <time.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

struct message {
    int value1;
    int value2;
};

typedef struct {
    int num_worker;
    int num_tasks;
    int queue_size;
}options;

typedef struct {
    int worker_id;
    int pid;
    int task_processed;
    int total_effort;
}comp_msg;

int get_random_value() {
    return rand () % 10 + 1;
} //1 - 10 random value

void log_msg(char const * const format, ...)
{
    char time_buffer[9];
    time_t const now = time(NULL);
    strftime(time_buffer, sizeof(time_buffer),"%H:%M:%S",localtime(&now));
    printf("[%s]", time_buffer);

    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

options parse_command_line(int const argc, char *argv[]){

    char const *const usage = "Usage %s -w <num_workers> -t <num_tasks>  -s <queue_size>\n";
    options opts = {0};

    int code = -1;
    do {
        code = getopt(argc, argv, "w:t:s:");
        switch(code) {
            case 'w':
                opts.num_worker = atoi(optarg);
                break;
            case 't':
                opts.num_tasks = atoi(optarg);
                break;
            case 's':
                opts.queue_size = atoi(optarg);
                break;
            case '?':
                fprintf(stderr, usage, argv[0]);
                exit(EXIT_FAILURE);
        }
    }while(code != -1);

    if  (opts.num_worker <= 0 || opts.num_tasks <= 0 || opts.queue_size <= 0){
        fprintf(stderr, usage, argv[0]);
        exit(EXIT_FAILURE);
    }

    return opts;
}

void run_worker(int worker) {

    mqd_t snek_q = mq_open("/snek", O_RDONLY);

    int worker_id = worker;
    int pid = getpid();
    int processed_task = 0;
    int total_effort = 0;

    while(1){
        struct message msg = {0};
        char message[128] = {0};

        int recvd;
        do {
            recvd = mq_receive(snek_q, (char*)&msg, sizeof(struct message), NULL);
        } while (recvd == -1 && errno == EINTR);

        if (msg.value1 == 0) {
            log_msg(" | Worker  #%d | Received termination task\n", worker);
            break;
        }

        ++processed_task;

        log_msg(" | Worker  #%d | Received task with effort %d\n", worker, msg.value2);

        sleep(msg.value2);
        total_effort += msg.value2;
    }

    mqd_t comp_q = mq_open("/comp", O_WRONLY);
    comp_msg stats = {
        worker_id,
        pid,
        processed_task,
        total_effort
    };

    int sent;
    do {
        sent = mq_send(comp_q, (char*)&stats, sizeof(comp_msg), 0);
    } while (sent == -1 && errno == EINTR);

    //simulation some work
    mq_close(snek_q);
    mq_close(comp_q);
    exit(EXIT_SUCCESS);

}

int main(int argc, char * argv[]){
    srand(time(NULL));

    options const opts = parse_command_line(argc, argv);

    log_msg(" | Dispatcher | Starting %d workers for %d tasks with queue size %d\n",
        opts.num_worker, opts.num_tasks, opts.queue_size);

    struct mq_attr queue_settings = {
        .mq_maxmsg = opts.queue_size,
        .mq_msgsize = sizeof(struct message)
    };

    struct mq_attr comp_queue_settings = {
        .mq_maxmsg = opts.num_worker,
        .mq_msgsize = sizeof(comp_msg)
    };

    mq_unlink("/snek");
    mqd_t snek_q = mq_open("/snek", O_WRONLY | O_CREAT | O_EXCL, 0600, &queue_settings);

    mq_unlink("/comp");
    mqd_t comp_q = mq_open("/comp", O_RDONLY | O_CREAT | O_EXCL, 0600, &comp_queue_settings);

    for (int i = 1; i <= opts.num_worker; ++i){
        pid_t const pid = fork();
        if (pid == 0) {
            run_worker(i);
        }
        log_msg(" | Worker  #%d | Started worker PID %d\n", i, pid);
    }

    log_msg(" | Dispatcher | Distributing tasks\n");

    for (int i = 1; i <= opts.num_tasks; ++i){
        int effort = get_random_value();
        struct message msg = {.value1 = i, .value2 = effort};
        log_msg(" | Dispatcher | Queuing task #%d with effort %d\n", msg.value1, msg.value2);
        int sent;
        do {
            sent = mq_send(snek_q, (char *)&msg, sizeof(struct message), 0);
        }while (sent == -1 && errno == EINTR);
    }

    log_msg(" | Dispatcher | Sending termination task\n");

    for (int i = 1; i <= opts.num_worker; ++i){
        struct message msg = {.value1 = 0};
        int sent;
        do {
            sent = mq_send(snek_q, (char*)&msg, sizeof(struct message), 0);
        } while (sent == -1 && errno == EINTR);
    }
    log_msg(" | Dispatcher | Waiting for workers to terminate\n");
    for(int i = 1; i <= opts.num_worker; i++) {
        comp_msg comp_msg = {0};

        int recvd;
        do {
            recvd = mq_receive(comp_q, (char*)&comp_msg, sizeof(comp_msg), NULL);
        } while (recvd == -1 && errno == EINTR);

        log_msg(" | Dispatcher | Worker #%d processed %d tasks in %d seconds\n", comp_msg.worker_id, comp_msg.task_processed, comp_msg.total_effort);
        log_msg(" | Dispatcher | Worker #%d with PID %d exited with status 0\n", comp_msg.worker_id, comp_msg.pid);
    }

    for(int i = 1; i <= opts.num_worker; i++) {
        wait(NULL);
    }

    mq_unlink("/comp");
    mq_close(comp_q);
    mq_close(snek_q);
    mq_unlink("/snek");

    return 0;
}