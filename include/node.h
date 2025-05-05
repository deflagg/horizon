#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#define MAX_CONNECTION_COUNT 10

struct Node
{
    int defaultBufferLength;
    const char *defaultIp;
    const char *defaultPort;
    int connections[MAX_CONNECTION_COUNT];
    FD_SET readSet;
    FD_SET writeSet;
    FD_SET errorSet;
    struct addrinfo *result;
    WSADATA wsaData;

    int (*Start)(struct Node *self, const char *ip, const char *port);
    
};

struct Node NodeConstructor();