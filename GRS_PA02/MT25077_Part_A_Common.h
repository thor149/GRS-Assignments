/* MT25077_Part_A_Common.h - Shared definitions and utilities */

#define _GNU_SOURCE

#ifndef MT25077_COMMON_H
#define MT25077_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <poll.h>

#define NUM_FIELDS        8   /* 8 dynamically allocated string fields per message */
#define DEFAULT_PORT      9000
#define DEFAULT_DURATION  10  /* seconds per experiment */

/* Message with NUM_FIELDS heap-allocated fields; total size = field_size * NUM_FIELDS */
struct message {
    char *fields[NUM_FIELDS];
};

struct server_thread_args {
    int    client_fd;
    int    msg_size;
    int    field_size;
};

struct client_thread_args {
    const char *server_ip;
    int         port;
    int         msg_size;
    int         duration;
    int         thread_id;
    /* Results written by thread, read by main after join */
    long long   bytes_received;
    long long   messages_received;
    double      total_latency_us;
};

/* Returns monotonic time in microseconds */
static inline double get_time_us(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1e6 + (double)ts.tv_nsec / 1e3;
}

/* Allocate message with NUM_FIELDS fields, each filled with pattern data */
static inline struct message *alloc_message(int msg_size) {
    struct message *msg = (struct message *)malloc(sizeof(struct message));
    if (!msg) {
        perror("malloc message");
        return NULL;
    }
    int field_size = msg_size / NUM_FIELDS;
    for (int i = 0; i < NUM_FIELDS; i++) {
        msg->fields[i] = (char *)malloc(field_size);
        if (!msg->fields[i]) {
            perror("malloc field");
            for (int j = 0; j < i; j++) free(msg->fields[j]);
            free(msg);
            return NULL;
        }
        memset(msg->fields[i], 'A' + i, field_size);
    }
    return msg;
}

static inline void free_message(struct message *msg) {
    if (!msg) return;
    for (int i = 0; i < NUM_FIELDS; i++) {
        free(msg->fields[i]);
    }
    free(msg);
}

/* Send exactly len bytes, handling partial sends */
static inline ssize_t send_all(int fd, const void *buf, size_t len) {
    size_t total = 0;
    while (total < len) {
        ssize_t n = send(fd, (const char *)buf + total, len - total, 0);
        if (n <= 0) return n;
        total += (size_t)n;
    }
    return (ssize_t)total;
}

/* Receive exactly len bytes, handling partial reads */
static inline ssize_t recv_all(int fd, void *buf, size_t len) {
    size_t total = 0;
    while (total < len) {
        ssize_t n = recv(fd, (char *)buf + total, len - total, 0);
        if (n <= 0) return n;
        total += (size_t)n;
    }
    return (ssize_t)total;
}

/* Uses sigaction without SA_RESTART so accept() returns EINTR */
static inline void install_signal_handler(int signum, void (*handler)(int)) {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(signum, &sa, NULL);
}

/* Create, bind, and listen on a TCP socket. Returns fd or -1 */
static inline int create_server_socket(int port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket");
        return -1;
    }

    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(fd);
        return -1;
    }

    if (listen(fd, 128) < 0) {
        perror("listen");
        close(fd);
        return -1;
    }

    return fd;
}

/* Connect to server, disable Nagle for latency measurement */
static inline int connect_to_server(const char *server_ip, int port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket");
        return -1;
    }

    int flag = 1;
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(port);
    if (inet_pton(AF_INET, server_ip, &addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(fd);
        return -1;
    }

    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("connect");
        close(fd);
        return -1;
    }

    return fd;
}

#endif /* MT25077_COMMON_H */
