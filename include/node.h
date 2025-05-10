#define _POSIX_C_SOURCE 200112L
#ifndef NODE_H
#define NODE_H

/* Public interface – put in include/node.h if you prefer split files */
#include <sys/select.h>
#include <netdb.h>
#include <netinet/in.h>

enum { MAX_CONNECTIONS = 10, BUF_SIZE = 512 };

typedef struct {
    const char *listen_port;
    const char *peer_port;
    int listen_sock;
    int conns[MAX_CONNECTIONS];
} Node;

int  create_listener(const char *port);
int  connect_peer  (const char *ip, const char *port);
int  add_conn      (Node *n, int fd);
void event_loop    (Node *n);

#endif