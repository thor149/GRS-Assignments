/*
 * MT25077_Part_B_workers.c
 *
 * Implementation of worker functions: CPU-intensive, Memory-intensive, and I/O-intensive
 *
 * Roll Number: MT25077
 * Loop Count: 7 × 1000 = 7000 iterations
 *
 * These worker functions are designed to stress different system resources:
 * - worker_cpu(): Stresses the CPU with mathematical calculations
 * - worker_mem(): Stresses memory subsystem with large allocations and operations
 * - worker_io(): Stresses disk I/O with file read/write operations
 */

#include "MT25077_Part_B_workers.h"

/*
 * Helper function for qsort in memory-intensive worker
 * Compares two integers for sorting
 */
int compare_int(const void *a, const void *b) {
    int int_a = *((int *)a);
    int int_b = *((int *)b);

    if (int_a < int_b) return -1;
    else if (int_a > int_b) return 1;
    else return 0;
}

/*
 * CPU-Intensive Worker Function
 *
 * This function performs computationally expensive operations to maximize CPU usage.
 * It executes complex mathematical calculations including:
 * - Trigonometric functions (sin, cos, tan, asin, acos, atan)
 * - Power operations
 * - Square root calculations
 * - Logarithmic calculations
 * - Exponential calculations
 *
 * The goal is to keep the CPU cores busy with continuous computation,
 * minimizing time spent waiting for memory or I/O.
 */
void worker_cpu(void) {
    double result = 0.0;
    double temp = 0.0;

    /* Main computation loop - 7000 iterations */
    for (long i = 1; i < LOOP_COUNT + 1; i++) {
        /* Trigonometric calculations - CPU intensive */
        temp = sin((double)i);
        temp += cos((double)i);
        temp += tan((double)i / 1000.0);  /* Scaled to avoid overflow */

        /* Inverse trigonometric functions */
        temp += asin(sin((double)i / LOOP_COUNT));
        temp += acos(cos((double)i / LOOP_COUNT));
        temp += atan((double)i);

        /* Power operations - CPU intensive */
        temp += pow((double)i, 2.5);
        temp += pow((double)i, 1.5);
        temp /= pow((double)(i + 1), 0.5);

        /* Square root calculations */
        temp += sqrt((double)i * 1.5);
        temp += sqrt((double)(i * i + 1));

        /* Logarithmic calculations */
        temp += log((double)i + 1);
        temp += log10((double)i + 1);

        /* Exponential calculations (scaled to prevent overflow) */
        temp += exp((double)i / LOOP_COUNT);

        /* Accumulate result to prevent compiler optimization */
        result += temp / (double)(i + 1);

        /* Additional nested computation to increase CPU load */
        /* Using CPU_INTENSIVE_MULTIPLIER to ensure long enough CPU burst for measurement */
        for (int j = 0; j < CPU_INTENSIVE_MULTIPLIER; j++) {
            temp = sqrt((double)(i + j)) * sin((double)j);
            result += temp / (double)(i + j + 1);
        }
    }

    /* Print result to prevent compiler from optimizing away the entire computation */
    if (result > 0) {
        /* Result computed successfully */
    }
}

/*
 * Memory-Intensive Worker Function
 *
 * This function stresses the memory subsystem by:
 * - Allocating large blocks of memory (multiple MB per iteration)
 * - Initializing arrays with pseudo-random data
 * - Performing memory-intensive sorting operations
 * - Accessing memory in non-sequential patterns to stress cache
 * - Copying large blocks of memory
 *
 * The goal is to create memory pressure and cause frequent cache misses,
 * making the memory subsystem the bottleneck rather than the CPU.
 */
void worker_mem(void) {
    /* Size of array to allocate per iteration (in integers)
     * Approximately 5 MB per iteration (5 * 256 * 1024 integers = 5MB)
     */
    const size_t array_size = 256 * 1024;  /* 1 MB per iteration */

    /* Main memory operations loop - 7000 iterations */
    for (long i = 0; i < LOOP_COUNT; i++) {
        /* Allocate large array - causes memory pressure */
        int *arr = (int *)malloc(array_size * sizeof(int));
        if (arr == NULL) {
            fprintf(stderr, "Memory allocation failed at iteration %ld\n", i);
            continue;
        }

        /* Fill array with pseudo-random data - memory write intensive */
        for (size_t j = 0; j < array_size; j++) {
            arr[j] = (int)((i * array_size + j) % 1000000);
        }

        /* Access memory in non-sequential pattern - causes cache misses */
        long sum = 0;
        for (size_t j = 0; j < array_size; j += 64) {  /* Jump by cache line size */
            sum += arr[j];
        }

        /* Sort array - extremely memory intensive operation */
        qsort(arr, array_size, sizeof(int), compare_int);

        /* Allocate second array for copying */
        int *arr2 = (int *)malloc(array_size * sizeof(int));
        if (arr2 != NULL) {
            /* Memory copy operation - memory bandwidth intensive */
            memcpy(arr2, arr, array_size * sizeof(int));

            /* Access copied array to ensure operation completed */
            volatile long checksum = 0;
            for (size_t j = 0; j < array_size; j += 128) {
                checksum += arr2[j];
            }

            /* Free second array */
            free(arr2);
        }

        /* Free allocated memory */
        free(arr);
    }
}

/*
 * I/O-Intensive Worker Function
 *
 * This function stresses the I/O subsystem by:
 * - Creating temporary files unique to each process/thread
 * - Writing substantial amounts of data to disk
 * - Reading data back from disk
 * - Performing sync operations to ensure data is written
 *
 * The goal is to make the process spend most of its time waiting for
 * disk I/O operations to complete, rather than performing CPU computation.
 */
void worker_io(void) {
    /* Create unique filename based on process/thread ID and timestamp */
    char filename[512];
    snprintf(filename, sizeof(filename), "/tmp/io_test_%d_%ld.dat",
             getpid(), (long)time(NULL));

    /* Buffer for I/O operations - 64 KB per operation */
    const size_t buffer_size = 64 * 1024;
    char *buffer = (char *)malloc(buffer_size);

    if (buffer == NULL) {
        fprintf(stderr, "Failed to allocate I/O buffer\n");
        return;
    }

    /* Fill buffer with data pattern */
    for (size_t i = 0; i < buffer_size; i++) {
        buffer[i] = (char)(i % 256);
    }

    /* Main I/O loop - 7000 iterations */
    for (long i = 0; i < LOOP_COUNT; i++) {
        /* WRITE OPERATION - Open file for writing */
        FILE *fp = fopen(filename, "w");
        if (fp == NULL) {
            fprintf(stderr, "Failed to open file for writing: %s\n", filename);
            continue;
        }

        /* Write multiple blocks to ensure substantial I/O */
        for (int j = 0; j < 16; j++) {  /* Write 16 * 64KB = 1MB per iteration */
            size_t written = fwrite(buffer, 1, buffer_size, fp);
            if (written != buffer_size) {
                fprintf(stderr, "Write error at iteration %ld\n", i);
            }
        }

        /* Flush to ensure data is written to disk */
        fflush(fp);
        fclose(fp);

        /* READ OPERATION - Open file for reading */
        fp = fopen(filename, "r");
        if (fp == NULL) {
            fprintf(stderr, "Failed to open file for reading: %s\n", filename);
            continue;
        }

        /* Read back the data */
        for (int j = 0; j < 16; j++) {  /* Read 16 * 64KB = 1MB per iteration */
            size_t read_bytes = fread(buffer, 1, buffer_size, fp);
            if (read_bytes != buffer_size && !feof(fp)) {
                fprintf(stderr, "Read error at iteration %ld\n", i);
            }
        }

        fclose(fp);
    }

    /* Cleanup: Remove temporary file */
    if (unlink(filename) != 0) {
        fprintf(stderr, "Warning: Failed to delete temporary file: %s\n", filename);
    }

    /* Free buffer */
    free(buffer);
}
