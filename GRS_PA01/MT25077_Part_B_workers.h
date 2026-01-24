/*
 * MT25077_Part_B_workers.h
 *
 * Header file for worker functions: CPU-intensive, Memory-intensive, and I/O-intensive
 *
 * Roll Number: MT25077
 * Loop Count: 7 × 1000 = 7000 iterations
 *
 * This file declares three worker functions that can be executed by both
 * processes (via fork) and threads (via pthread).
 */

#ifndef MT25077_PART_B_WORKERS_H
#define MT25077_PART_B_WORKERS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>

/* Loop iteration count based on roll number last digit (7) */
#define LOOP_COUNT 7000

/* CPU-intensive worker needs more iterations for measurable CPU burst */
#define CPU_INTENSIVE_MULTIPLIER 10000000  /* Makes CPU worker run 30-60 seconds */

/*
 * CPU-intensive worker function
 * Performs heavy mathematical calculations including trigonometric functions,
 * power operations, and square roots to maximize CPU usage.
 */
void worker_cpu(void);

/*
 * Memory-intensive worker function
 * Allocates large memory blocks, initializes them, performs sorting operations,
 * and accesses memory in patterns that stress the memory subsystem and cache.
 */
void worker_mem(void);

/*
 * I/O-intensive worker function
 * Performs substantial disk read/write operations to temporary files,
 * causing the process to wait on I/O operations.
 */
void worker_io(void);

/*
 * Helper function for memory worker (qsort comparison)
 */
int compare_int(const void *a, const void *b);

#endif /* MT25077_PART_B_WORKERS_H */
