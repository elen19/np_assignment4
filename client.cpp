#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <iostream>
#include <signal.h>
#define DEBUG
#define PROTOCOL "RPS TCP 1.0\n"

void intSignal(int sig)
{
    printf("\n");
    exit(0);
}
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Wrong format IP:PORT\n");
        exit(0);
    }

    char delim[] = ":";
    char *Desthost = strtok(argv[1], delim);
    char *Destport = strtok(NULL, delim);

    if (Desthost == NULL || Destport == NULL)
    {
        printf("Wrong format\n");
        exit(0);
    }
    addrinfo sa, *si, *p;
    memset(&sa, 0, sizeof(sa));
    sa.ai_family = AF_INET;
    sa.ai_socktype = SOCK_STREAM;
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    if (int rv = getaddrinfo(Desthost, Destport, &sa, &si) != 0)
    {
        fprintf(stderr, "%s\n", gai_strerror(rv));
        exit(0);
    }

    int sockfd;
    for (p = si; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            continue;
        }
        if ((connect(sockfd, p->ai_addr, p->ai_addrlen)) != 0)
        {
            close(sockfd);
            printf("Couldn't connect to server.\n");
            continue;
        }
        break;
    }
    if (p == NULL)
    {
        printf("Couldn't create connect.\n");
        exit(0);
    }
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv));
    freeaddrinfo(si);

    char recvBuf[256];
    int bytes;
    fd_set currentSockets;
    fd_set readySockets;
    FD_ZERO(&currentSockets);
    FD_ZERO(&readySockets);
    FD_SET(sockfd, &currentSockets);
    FD_SET(STDIN_FILENO, &currentSockets);
    int fdMax = sockfd;
    int nfds = 0;
    char writeBuf[5];
    signal(SIGINT, intSignal);
    while (true)
    {
        readySockets = currentSockets;
        if (fdMax < sockfd)
        {
            fdMax = sockfd;
        }
        nfds = select(fdMax + 1, &readySockets, NULL, NULL, NULL);
        if (nfds == -1)
        {
            printf("Something went wrong with select\n");
            break;
        }
        if (FD_ISSET(STDIN_FILENO, &readySockets))
        {
            memset(writeBuf, 0, sizeof(writeBuf));
            std::cin.getline(writeBuf, sizeof(writeBuf));
            std::cin.clear();
            if (strstr(writeBuf, "0") != nullptr)
            {
                FD_CLR(STDIN_FILENO, &readySockets);
                exit(0);
            }
            else
            {
                send(sockfd, writeBuf, strlen(writeBuf), 0);
                FD_CLR(STDIN_FILENO, &readySockets);
            }
        }
        if (FD_ISSET(sockfd, &readySockets))
        {
            memset(recvBuf, 0, sizeof(recvBuf));
            bytes = recv(sockfd, recvBuf, sizeof(recvBuf), 0);
            if (bytes == -1)
            {
                printf("Failed to recive. \n");
                continue;
            }
            else if (strstr(recvBuf, PROTOCOL) != nullptr)
            {
                printf("Serverprotocol accepted, Protocol: %s", recvBuf);
                send(sockfd, "OK\n", strlen("OK\n"), 0);
            }
            else if (strstr(recvBuf, "SERVER CLOSED") != nullptr)
            {
                printf("%s", recvBuf);
                exit(0);
            }
            else if (strstr(recvBuf, "ERROR") != nullptr)
            {
                printf("%s", recvBuf);
            }
            else
            {
                printf("%s", recvBuf);
            }
            FD_CLR(sockfd, &readySockets);
        }
    }
    close(sockfd);
    return 0;
}