/* MT25077_Part_A3_Server.c - Zero-Copy TCP Server (MSG_ZEROCOPY) */

#include "MT25077_Part_A_Common.h"
#include <linux/errqueue.h>

#ifndef SO_ZEROCOPY
#define SO_ZEROCOPY 60
#endif

#ifndef MSG_ZEROCOPY
#define MSG_ZEROCOPY 0x4000000
#endif

#ifndef SO_EE_ORIGIN_ZEROCOPY
#define SO_EE_ORIGIN_ZEROCOPY 5
#endif

static volatile int g_running = 1;

static void signal_handler(int sig) {
    (void)sig;
    g_running = 0;
}

/* Drain zero-copy completion notifications from the error queue */
static void drain_completions(int fd) {
    char cbuf[CMSG_SPACE(sizeof(struct sock_extended_err))];
    struct msghdr msg;
    struct iovec iov_dummy;
    char dummy;

    iov_dummy.iov_base = &dummy;
    iov_dummy.iov_len  = 0;

    while (1) {
        memset(&msg, 0, sizeof(msg));
        msg.msg_control    = cbuf;
        msg.msg_controllen = sizeof(cbuf);
        msg.msg_iov        = &iov_dummy;
        msg.msg_iovlen     = 1;

        int ret = recvmsg(fd, &msg, MSG_ERRQUEUE | MSG_DONTWAIT);
        if (ret < 0) break;
    }
}

static void *client_handler(void *arg) {
    struct server_thread_args *ta = (struct server_thread_args *)arg;
    int msg_size   = ta->msg_size;
    int field_size = ta->field_size;
    int client_fd  = ta->client_fd;
    free(ta);

    /* Enable zero-copy on this socket */
    int val = 1;
    if (setsockopt(client_fd, SOL_SOCKET, SO_ZEROCOPY,
                   &val, sizeof(val)) < 0) {
        perror("setsockopt SO_ZEROCOPY");
        fprintf(stderr, "[A3-Server] Warning: zero-copy not supported, "
                "falling back to normal send.\n");
    }

    struct message *msg = alloc_message(msg_size);
    if (!msg) {
        close(client_fd);
        return NULL;
    }

    /* iovec: scatter-gather from each heap field */
    struct iovec iov[NUM_FIELDS];
    for (int i = 0; i < NUM_FIELDS; i++) {
        iov[i].iov_base = msg->fields[i];
        iov[i].iov_len  = field_size;
    }

    struct msghdr mhdr;
    memset(&mhdr, 0, sizeof(mhdr));
    mhdr.msg_iov    = iov;
    mhdr.msg_iovlen = NUM_FIELDS;

    long long send_count = 0;

    while (g_running) {
        /* Zero-copy: kernel pins user pages, NIC DMAs from them */
        ssize_t ret = sendmsg(client_fd, &mhdr, MSG_ZEROCOPY);

        if (ret < 0) {
            if (errno == ENOBUFS) {
                /* Too many outstanding buffers, drain and retry */
                drain_completions(client_fd);
                continue;
            }
            break;
        }

        send_count++;

        /* Drain completions every 64 sends to free buffers */
        if ((send_count & 63) == 0) {
            drain_completions(client_fd);
        }
    }

    drain_completions(client_fd);

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

    fprintf(stderr, "[A3-Server] Zero-copy server listening on port %d, "
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
    fprintf(stderr, "[A3-Server] Shutdown complete.\n");
    return 0;
}
