#define _POSIX_C_SOURCE 200112L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <arpa/inet.h>

#include "../../include/node.h"

static int make_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int add_conn(Node *n, int fd) {
    for (int i = 0; i < MAX_CONNECTIONS; ++i) {
        if (n->conns[i] == -1) {
            n->conns[i] = fd;
            return 0;
        }
    }
    return -1; /* table full */
}

static void drop_conn(Node *n, int fd) {
    for (int i = 0; i < MAX_CONNECTIONS; ++i) {
        if (n->conns[i] == fd) {
            close(fd);
            n->conns[i] = -1;
            return;
        }
    }
}

int create_listener(const char *port) {
    struct addrinfo hints = {0}, *ai, *p;
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags    = AI_PASSIVE;

    if (getaddrinfo(NULL, port, &hints, &ai) != 0) {
        perror("getaddrinfo");
        return -1;
    }

    int s = -1;
    for (p = ai; p; p = p->ai_next) {
        s = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (s == -1) continue;
        int opt = 1;
        setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        if (bind(s, p->ai_addr, p->ai_addrlen) == 0) break;
        close(s);
        s = -1;
    }
    freeaddrinfo(ai);
    if (s == -1) { perror("bind"); return -1; }
    if (listen(s, SOMAXCONN) == -1) { perror("listen"); close(s); return -1; }
    make_nonblocking(s);
    return s;
}

int connect_peer(const char *ip, const char *port) {
    struct addrinfo hints = {0}, *ai, *p;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    if (getaddrinfo(ip, port, &hints, &ai) != 0) {
        perror("getaddrinfo peer");
        return -1;
    }
    int s = -1;
    for (p = ai; p; p = p->ai_next) {
        s = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (s == -1) continue;
        if (connect(s, p->ai_addr, p->ai_addrlen) == 0) break;
        close(s);
        s = -1;
    }
    freeaddrinfo(ai);
    if (s != -1) make_nonblocking(s);
    return s;
}

void event_loop(Node *n) {
    fd_set rset;
    char buf[BUF_SIZE];

    for (;;) {
        FD_ZERO(&rset);
        FD_SET(n->listen_sock, &rset);
        int maxfd = n->listen_sock;

        FD_SET(STDIN_FILENO, &rset);
        if (STDIN_FILENO > maxfd) maxfd = STDIN_FILENO;

        for (int i = 0; i < MAX_CONNECTIONS; ++i) {
            int fd = n->conns[i];
            if (fd != -1) {
                FD_SET(fd, &rset);
                if (fd > maxfd) maxfd = fd;
            }
        }

        if (select(maxfd + 1, &rset, NULL, NULL, NULL) == -1) {
            if (errno == EINTR) continue;
            perror("select");
            break;
        }

        if (FD_ISSET(n->listen_sock, &rset)) {
            struct sockaddr_in cli; socklen_t len = sizeof(cli);
            int c = accept(n->listen_sock, (struct sockaddr *)&cli, &len);
            if (c != -1) {
                make_nonblocking(c);
                add_conn(n, c);
                printf("[+] Accepted %s:%d (fd=%d)\n", inet_ntoa(cli.sin_addr), ntohs(cli.sin_port), c);
            }
        }

        if (FD_ISSET(STDIN_FILENO, &rset)) {
            if (fgets(buf, sizeof buf, stdin)) {
                size_t len = strlen(buf);
                for (int i = 0; i < MAX_CONNECTIONS; ++i) {
                    int fd = n->conns[i];
                    if (fd != -1) send(fd, buf, len, 0);
                }
            }
        }

        for (int i = 0; i < MAX_CONNECTIONS; ++i) {
            int fd = n->conns[i];
            if (fd != -1 && FD_ISSET(fd, &rset)) {
                ssize_t nread = recv(fd, buf, sizeof buf - 1, 0);
                if (nread <= 0) {
                    printf("[-] Closing fd=%d\n", fd);
                    drop_conn(n, fd);
                } else {
                    buf[nread] = '\0';
                    printf("[peer %d] %s", fd, buf);
                }
            }
        }
    }
}
