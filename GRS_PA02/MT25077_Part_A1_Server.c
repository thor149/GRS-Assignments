/* MT25077_Part_A1_Server.c - Two-Copy TCP Server (Baseline) */

#include "MT25077_Part_A_Common.h"

static volatile int g_running = 1;

static void signal_handler(int sig) {
    (void)sig;
    g_running = 0;
}

static void *client_handler(void *arg) {
    struct server_thread_args *ta = (struct server_thread_args *)arg;
    int msg_size   = ta->msg_size;
    int field_size = ta->field_size;
    int client_fd  = ta->client_fd;
    free(ta);

    struct message *msg = alloc_message(msg_size);
    if (!msg) {
        close(client_fd);
        return NULL;
    }

    /* Contiguous send buffer for serialization (Copy 1) */
    char *send_buf = (char *)malloc(msg_size);
    if (!send_buf) {
        perror("malloc send_buf");
        free_message(msg);
        close(client_fd);
        return NULL;
    }

    while (g_running) {
        /* Copy 1: serialize fields into contiguous buffer */
        for (int i = 0; i < NUM_FIELDS; i++)
            memcpy(send_buf + i * field_size, msg->fields[i], field_size);

        /* Copy 2: send() copies send_buf into kernel socket buffer */
        ssize_t ret = send_all(client_fd, send_buf, msg_size);
        if (ret <= 0) break;
    }

    free(send_buf);
    free_message(msg);
    close(client_fd);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <port> <message_size_bytes>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int port     = atoi(argv[1]);
    int msg_size = atoi(argv[2]);

    msg_size = (msg_size / NUM_FIELDS) * NUM_FIELDS;
    if (msg_size == 0) {
        fprintf(stderr, "Error: message size must be >= %d bytes\n", NUM_FIELDS);
        return EXIT_FAILURE;
    }
    int field_size = msg_size / NUM_FIELDS;

    install_signal_handler(SIGINT,  signal_handler);
    install_signal_handler(SIGTERM, signal_handler);
    install_signal_handler(SIGPIPE, SIG_IGN);

    int server_fd = create_server_socket(port);
    if (server_fd < 0) return EXIT_FAILURE;

    fprintf(stderr, "[A1-Server] Two-copy server listening on port %d, "
            "msg_size=%d, field_size=%d\n", port, msg_size, field_size);

    while (g_running) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        int client_fd = accept(server_fd,
                               (struct sockaddr *)&client_addr, &client_len);
        if (client_fd < 0) {
            if (!g_running) break;
            perror("accept");
            continue;
        }

        struct server_thread_args *ta = (struct server_thread_args *)
            calloc(1, sizeof(struct server_thread_args));
        ta->client_fd  = client_fd;
        ta->msg_size   = msg_size;
        ta->field_size = field_size;

        pthread_t tid;
        if (pthread_create(&tid, NULL, client_handler, ta) != 0) {
            perror("pthread_create");
            free(ta);
            close(client_fd);
            continue;
        }
        pthread_detach(tid);
    }

    close(server_fd);
    fprintf(stderr, "[A1-Server] Shutdown complete.\n");
    return 0;
}
