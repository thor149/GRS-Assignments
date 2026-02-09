/* MT25077_Part_A1_Client.c - Two-Copy TCP Client (Baseline) */

#include "MT25077_Part_A_Common.h"

static void *client_thread_fn(void *arg) {
    struct client_thread_args *cta = (struct client_thread_args *)arg;
    int msg_size = cta->msg_size;

    int sock = connect_to_server(cta->server_ip, cta->port);
    if (sock < 0) return NULL;

    char *recv_buf = (char *)malloc(msg_size);
    if (!recv_buf) {
        perror("malloc recv_buf");
        close(sock);
        return NULL;
    }

    double start_time = get_time_us();
    double end_time   = start_time + (double)cta->duration * 1e6;
    double total_latency = 0.0;
    long long bytes = 0, msgs = 0;

    /* Receive loop: run for specified duration, measure per-msg latency */
    while (get_time_us() < end_time) {
        double msg_start = get_time_us();
        ssize_t ret = recv_all(sock, recv_buf, msg_size);
        if (ret <= 0) break;

        double msg_end = get_time_us();
        total_latency += (msg_end - msg_start);
        bytes += msg_size;
        msgs++;
    }

    double elapsed_s = (get_time_us() - start_time) / 1e6;

    cta->bytes_received    = bytes;
    cta->messages_received = msgs;
    cta->total_latency_us  = total_latency;

    fprintf(stderr, "  Thread %d: %.2f MB, %.4f Gbps, avg_lat=%.2f us\n",
            cta->thread_id,
            bytes / (1024.0 * 1024.0),
            (bytes * 8.0) / (elapsed_s * 1e9),
            msgs > 0 ? total_latency / msgs : 0.0);

    free(recv_buf);
    close(sock);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 5) {
        fprintf(stderr,
                "Usage: %s <server_ip> <port> <msg_size> <thread_count> "
                "[duration_s]\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *server_ip = argv[1];
    int port         = atoi(argv[2]);
    int msg_size     = atoi(argv[3]);
    int thread_count = atoi(argv[4]);
    int duration     = (argc > 5) ? atoi(argv[5]) : DEFAULT_DURATION;

    msg_size = (msg_size / NUM_FIELDS) * NUM_FIELDS;
    if (msg_size == 0) {
        fprintf(stderr, "Error: message size must be >= %d bytes\n", NUM_FIELDS);
        return EXIT_FAILURE;
    }

    fprintf(stderr, "[A1-Client] two_copy: server=%s:%d msg_size=%d "
            "threads=%d duration=%ds\n",
            server_ip, port, msg_size, thread_count, duration);

    pthread_t *threads = (pthread_t *)malloc(sizeof(pthread_t) * thread_count);
    struct client_thread_args *args = (struct client_thread_args *)
        calloc(thread_count, sizeof(struct client_thread_args));

    for (int i = 0; i < thread_count; i++) {
        args[i].server_ip = server_ip;
        args[i].port      = port;
        args[i].msg_size  = msg_size;
        args[i].duration  = duration;
        args[i].thread_id = i;
        pthread_create(&threads[i], NULL, client_thread_fn, &args[i]);
    }

    long long total_bytes = 0, total_msgs = 0;
    double total_latency = 0.0;

    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
        total_bytes   += args[i].bytes_received;
        total_msgs    += args[i].messages_received;
        total_latency += args[i].total_latency_us;
    }

    double throughput_gbps = (total_bytes * 8.0) / ((double)duration * 1e9);
    double avg_latency_us  = total_msgs > 0
                             ? total_latency / (double)total_msgs
                             : 0.0;

    fprintf(stderr, "\n=== AGGREGATE RESULTS (two_copy) ===\n");
    fprintf(stderr, "Total bytes:  %lld\n", total_bytes);
    fprintf(stderr, "Total msgs:   %lld\n", total_msgs);
    fprintf(stderr, "Throughput:   %.4f Gbps\n", throughput_gbps);
    fprintf(stderr, "Avg latency:  %.2f us\n", avg_latency_us);

    /* Machine-parseable CSV line to stdout for experiment script */
    printf("RESULT,two_copy,%d,%d,%.4f,%.2f,%lld,%lld\n",
           msg_size, thread_count, throughput_gbps, avg_latency_us,
           total_bytes, total_msgs);

    free(threads);
    free(args);
    return 0;
}
