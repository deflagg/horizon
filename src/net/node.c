#undef UNICODE

#define WIN32_LEAN_AND_MEAN


#include "../../include/node.h"



int Start(struct Node *self, const char *ip, const char *port)
{
    //WSADATA wsaData;
    int iResult;

    // struct addrinfo *result = NULL;
    struct addrinfo hints;

    int iSendResult;
    
    char *recvbuf = malloc(self->defaultBufferLength);
    int recvbuflen = self->defaultBufferLength;
    
    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &self->wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, self->defaultPort, &hints, &self->result);
    if ( iResult != 0 ) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Create a SOCKET for the server to listen for client connections.
    SOCKET listenSocket = socket(self->result->ai_family, self->result->ai_socktype, self->result->ai_protocol);
    if (listenSocket == INVALID_SOCKET) {
        printf("socket failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(self->result);
        WSACleanup();
        return 1;
    }

    // Setup the TCP listening socket
    iResult = bind(listenSocket, self->result->ai_addr, (int)self->result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(self->result);
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(self->result);

    iResult = listen(listenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    self->connections[0] = listenSocket;

    while(1)
    {
        FD_ZERO(&self->readSet);
        FD_ZERO(&self->writeSet);
        FD_ZERO(&self->errorSet);

        FD_SET(listenSocket, &self->readSet);

        for(int i = 0; i < MAX_CONNECTION_COUNT; i++)
        {
            if(self->connections[i] > 0)
            {
                FD_SET(self->connections[i], &self->readSet);
            }
        }

        iResult = select(0, &self->readSet, &self->writeSet, &self->errorSet, NULL);

        if(iResult == SOCKET_ERROR)
        {
            printf("select() failed: %d\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }
        else if(iResult == 0)
        {
            printf("Timeout\n");
            return 1;
        }

        if(FD_ISSET(listenSocket, &self->readSet) > 0)
        {
            // Accept a client socket
            SOCKET clientSocket = accept(listenSocket, NULL, NULL);
            
            if (clientSocket == INVALID_SOCKET) {
                printf("accept failed with error: %d\n", WSAGetLastError());
                closesocket(listenSocket);
                WSACleanup();
                return 1;
            }

            for(int i = 0; i < MAX_CONNECTION_COUNT; i++)
            {
                if(self->connections[i] == 0)
                {
                    self->connections[i] == clientSocket;
                    break;
                }
            }
               
        }

        for (int i = 0; i < MAX_CONNECTION_COUNT; i++) 
        {
            SOCKET sd = self->connections[i];
            if (FD_ISSET(sd, &self->readSet)) 
            {
                char buffer[1024] = {0};
                int valread = recv(sd, buffer, 1024, 0);
                if (valread <= 0) 
                {
                    closesocket(sd);
                    self->connections[i] = 0;
                } 
                else 
                {
                    printf("Received: %s from socket %d\n", buffer, sd);
                    send(sd, buffer, strlen(buffer), 0);
                }
            }
        }
    }


    // No longer need server socket
    closesocket(listenSocket);
    WSACleanup();
    return 0;
}

struct Node NodeConstructor()
{
    struct Node node = 
    {
        .defaultBufferLength = 512,
        .defaultPort = "27015",
        .defaultIp = "127.0.0.1",
        .Start = Start
    };

    for(int i = 0; i < MAX_CONNECTION_COUNT; i++)
    {
        node.connections[i] = 0;
    }

    return node;
}