/*
 * MT25077_Part_A_Program_A.c
 *
 * Process-based program using fork() system call
 *
 * Roll Number: MT25077
 *
 * This program creates multiple child processes using fork() and executes
 * worker functions (CPU-intensive, Memory-intensive, or I/O-intensive)
 * in each child process.
 *
 * Usage: ./program_a <cpu|mem|io> [num_processes]
 *        - First argument specifies worker type: cpu, mem, or io
 *        - Second argument (optional) specifies number of child processes (default: 2)
 *
 * For Part C: ./program_a <worker_type>        (creates 2 child processes)
 * For Part D: ./program_a <worker_type> <N>    (creates N child processes)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include "MT25077_Part_B_workers.h"

/* Default number of child processes for Part C */
#define DEFAULT_NUM_PROCESSES 2

/* Function pointer type for worker functions */
typedef void (*worker_func_t)(void);

/*
 * Print usage information and exit
 */
void print_usage(const char *program_name) {
    fprintf(stderr, "Usage: %s <cpu|mem|io> [num_processes]\n", program_name);
    fprintf(stderr, "\n");
    fprintf(stderr, "Arguments:\n");
    fprintf(stderr, "  <cpu|mem|io>     - Type of worker function to execute:\n");
    fprintf(stderr, "                     'cpu' for CPU-intensive operations\n");
    fprintf(stderr, "                     'mem' for Memory-intensive operations\n");
    fprintf(stderr, "                     'io'  for I/O-intensive operations\n");
    fprintf(stderr, "  [num_processes]  - Number of child processes to create (default: 2)\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Examples:\n");
    fprintf(stderr, "  %s cpu           - Create 2 child processes running CPU worker\n", program_name);
    fprintf(stderr, "  %s mem 4         - Create 4 child processes running Memory worker\n", program_name);
    exit(EXIT_FAILURE);
}

/*
 * Parse worker type from command-line argument
 * Returns function pointer to the appropriate worker function
 */
worker_func_t get_worker_function(const char *worker_type) {
    if (strcmp(worker_type, "cpu") == 0) {
        return worker_cpu;
    } else if (strcmp(worker_type, "mem") == 0) {
        return worker_mem;
    } else if (strcmp(worker_type, "io") == 0) {
        return worker_io;
    } else {
        return NULL;
    }
}

/*
 * Main function
 */
int main(int argc, char *argv[]) {
    /* Check command-line arguments */
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Error: Invalid number of arguments\n\n");
        print_usage(argv[0]);
    }

    /* Parse worker type */
    const char *worker_type = argv[1];
    worker_func_t worker_func = get_worker_function(worker_type);

    if (worker_func == NULL) {
        fprintf(stderr, "Error: Invalid worker type '%s'\n", worker_type);
        fprintf(stderr, "Must be one of: cpu, mem, io\n\n");
        print_usage(argv[0]);
    }

    /* Parse number of processes (default: 2) */
    int num_processes = DEFAULT_NUM_PROCESSES;
    if (argc == 3) {
        num_processes = atoi(argv[2]);
        if (num_processes <= 0 || num_processes > 100) {
            fprintf(stderr, "Error: Invalid number of processes '%s'\n", argv[2]);
            fprintf(stderr, "Must be between 1 and 100\n");
            exit(EXIT_FAILURE);
        }
    }

    /* Display configuration */
    printf("=================================================================\n");
    printf("Program A: Process-based execution using fork()\n");
    printf("Roll Number: MT25077\n");
    printf("=================================================================\n");
    printf("Configuration:\n");
    printf("  Worker Type:       %s\n", worker_type);
    printf("  Number of Processes: %d child processes\n", num_processes);
    printf("  Loop Count per Worker: %d iterations\n", LOOP_COUNT);
    printf("  Parent PID:        %d\n", getpid());
    printf("=================================================================\n\n");

    /* Array to store child PIDs */
    pid_t *child_pids = (pid_t *)malloc(num_processes * sizeof(pid_t));
    if (child_pids == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for child PIDs\n");
        exit(EXIT_FAILURE);
    }

    /* Create child processes */
    printf("Creating %d child processes...\n", num_processes);
    for (int i = 0; i < num_processes; i++) {
        pid_t pid = fork();

        if (pid < 0) {
            /* Fork failed */
            fprintf(stderr, "Error: fork() failed for process %d: %s\n",
                    i, strerror(errno));

            /* Wait for any previously created children */
            for (int j = 0; j < i; j++) {
                waitpid(child_pids[j], NULL, 0);
            }

            free(child_pids);
            exit(EXIT_FAILURE);

        } else if (pid == 0) {
            /* Child process */
            printf("  Child %d: PID = %d, executing %s worker\n",
                   i + 1, getpid(), worker_type);
            fflush(stdout);

            /* Execute worker function */
            worker_func();

            /* Child exits successfully */
            exit(EXIT_SUCCESS);

        } else {
            /* Parent process: store child PID */
            child_pids[i] = pid;
        }
    }

    /* Parent process: wait for all children to complete */
    printf("\nParent (PID %d): Waiting for all %d child processes to complete...\n",
           getpid(), num_processes);

    int failed_count = 0;
    for (int i = 0; i < num_processes; i++) {
        int status;
        pid_t terminated_pid = waitpid(child_pids[i], &status, 0);

        if (terminated_pid < 0) {
            fprintf(stderr, "Error: waitpid() failed for PID %d: %s\n",
                    child_pids[i], strerror(errno));
            failed_count++;
        } else if (WIFEXITED(status)) {
            int exit_status = WEXITSTATUS(status);
            if (exit_status == 0) {
                printf("  Child PID %d completed successfully\n", terminated_pid);
            } else {
                fprintf(stderr, "  Child PID %d exited with status %d\n",
                        terminated_pid, exit_status);
                failed_count++;
            }
        } else if (WIFSIGNALED(status)) {
            fprintf(stderr, "  Child PID %d terminated by signal %d\n",
                    terminated_pid, WTERMSIG(status));
            failed_count++;
        }
    }

    /* Cleanup */
    free(child_pids);

    /* Print summary */
    printf("\n=================================================================\n");
    printf("Execution Summary:\n");
    printf("  Total child processes: %d\n", num_processes);
    printf("  Successful completions: %d\n", num_processes - failed_count);
    printf("  Failed processes: %d\n", failed_count);
    printf("=================================================================\n");

    /* Exit with appropriate status */
    if (failed_count > 0) {
        return EXIT_FAILURE;
    } else {
        return EXIT_SUCCESS;
    }
}
