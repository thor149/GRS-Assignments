/*
 * MT25077_Part_A_Program_B.c
 *
 * Thread-based program using pthread library
 *
 * Roll Number: MT25077
 *
 * This program creates multiple threads using pthread_create() and executes
 * worker functions (CPU-intensive, Memory-intensive, or I/O-intensive)
 * in each thread.
 *
 * Usage: ./program_b <cpu|mem|io> [num_threads]
 *        - First argument specifies worker type: cpu, mem, or io
 *        - Second argument (optional) specifies number of threads (default: 2)
 *
 * For Part C: ./program_b <worker_type>        (creates 2 threads)
 * For Part D: ./program_b <worker_type> <N>    (creates N threads)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include "MT25077_Part_B_workers.h"

/* Default number of threads for Part C */
#define DEFAULT_NUM_THREADS 2

/* Function pointer type for worker functions */
typedef void (*worker_func_t)(void);

/* Structure to pass data to thread */
typedef struct {
    int thread_id;
    worker_func_t worker_func;
    const char *worker_type;
} thread_data_t;

/*
 * Print usage information and exit
 */
void print_usage(const char *program_name) {
    fprintf(stderr, "Usage: %s <cpu|mem|io> [num_threads]\n", program_name);
    fprintf(stderr, "\n");
    fprintf(stderr, "Arguments:\n");
    fprintf(stderr, "  <cpu|mem|io>  - Type of worker function to execute:\n");
    fprintf(stderr, "                  'cpu' for CPU-intensive operations\n");
    fprintf(stderr, "                  'mem' for Memory-intensive operations\n");
    fprintf(stderr, "                  'io'  for I/O-intensive operations\n");
    fprintf(stderr, "  [num_threads] - Number of threads to create (default: 2)\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Examples:\n");
    fprintf(stderr, "  %s cpu        - Create 2 threads running CPU worker\n", program_name);
    fprintf(stderr, "  %s mem 4      - Create 4 threads running Memory worker\n", program_name);
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
 * Thread entry point
 * Each thread executes this function with its unique thread_data
 */
void *thread_function(void *arg) {
    thread_data_t *data = (thread_data_t *)arg;

    printf("  Thread %d: pthread_self = %lu, executing %s worker\n",
           data->thread_id, (unsigned long)pthread_self(), data->worker_type);
    fflush(stdout);

    /* Execute worker function */
    data->worker_func();

    /* Thread exits */
    pthread_exit(NULL);
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

    /* Parse number of threads (default: 2) */
    int num_threads = DEFAULT_NUM_THREADS;
    if (argc == 3) {
        num_threads = atoi(argv[2]);
        if (num_threads <= 0 || num_threads > 100) {
            fprintf(stderr, "Error: Invalid number of threads '%s'\n", argv[2]);
            fprintf(stderr, "Must be between 1 and 100\n");
            exit(EXIT_FAILURE);
        }
    }

    /* Display configuration */
    printf("=================================================================\n");
    printf("Program B: Thread-based execution using pthread\n");
    printf("Roll Number: MT25077\n");
    printf("=================================================================\n");
    printf("Configuration:\n");
    printf("  Worker Type:       %s\n", worker_type);
    printf("  Number of Threads: %d threads\n", num_threads);
    printf("  Loop Count per Worker: %d iterations\n", LOOP_COUNT);
    printf("  Main Thread ID:    %lu\n", (unsigned long)pthread_self());
    printf("  Process PID:       %d\n", getpid());
    printf("=================================================================\n\n");

    /* Allocate arrays for thread handles and thread data */
    pthread_t *threads = (pthread_t *)malloc(num_threads * sizeof(pthread_t));
    thread_data_t *thread_data = (thread_data_t *)malloc(num_threads * sizeof(thread_data_t));

    if (threads == NULL || thread_data == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for threads\n");
        free(threads);
        free(thread_data);
        exit(EXIT_FAILURE);
    }

    /* Create threads */
    printf("Creating %d threads...\n", num_threads);
    int created_count = 0;
    for (int i = 0; i < num_threads; i++) {
        /* Initialize thread data */
        thread_data[i].thread_id = i + 1;
        thread_data[i].worker_func = worker_func;
        thread_data[i].worker_type = worker_type;

        /* Create thread */
        int result = pthread_create(&threads[i], NULL, thread_function, &thread_data[i]);

        if (result != 0) {
            /* Thread creation failed */
            fprintf(stderr, "Error: pthread_create() failed for thread %d: %s\n",
                    i + 1, strerror(result));

            /* Wait for previously created threads */
            for (int j = 0; j < i; j++) {
                pthread_join(threads[j], NULL);
            }

            free(threads);
            free(thread_data);
            exit(EXIT_FAILURE);
        }

        created_count++;
    }

    /* Main thread: wait for all threads to complete */
    printf("\nMain thread: Waiting for all %d threads to complete...\n", num_threads);

    int failed_count = 0;
    for (int i = 0; i < num_threads; i++) {
        void *thread_return;
        int result = pthread_join(threads[i], &thread_return);

        if (result != 0) {
            fprintf(stderr, "Error: pthread_join() failed for thread %d: %s\n",
                    i + 1, strerror(result));
            failed_count++;
        } else {
            printf("  Thread %d completed successfully\n", i + 1);
        }
    }

    /* Cleanup */
    free(threads);
    free(thread_data);

    /* Print summary */
    printf("\n=================================================================\n");
    printf("Execution Summary:\n");
    printf("  Total threads: %d\n", num_threads);
    printf("  Successful completions: %d\n", num_threads - failed_count);
    printf("  Failed threads: %d\n", failed_count);
    printf("=================================================================\n");

    /* Exit with appropriate status */
    if (failed_count > 0) {
        return EXIT_FAILURE;
    } else {
        return EXIT_SUCCESS;
    }
}
