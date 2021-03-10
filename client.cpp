#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#include <iostream>
#define DEBUG
#define PROTOCOL "RPS UDP 1.0\n"
void intSignal(int sig)
{
    printf("\n");
    exit(0);
}
int main(int argc, char *argv[])
{

    if (argc != 2)
    {
        printf("Wrong format.\n");
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
    int port = atoi(Destport);
    addrinfo sa, *si, *p;
    memset(&sa, 0, sizeof(sa));
    sa.ai_family = AF_INET;
    sa.ai_socktype = SOCK_DGRAM;
    sa.ai_protocol = 17;
    int sockfd;
    int rv;
    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    if ((rv = getaddrinfo(Desthost, Destport, &sa, &si)) != 0)
    {
        fprintf(stderr, "getadrrinfo: %s\n", gai_strerror(rv));
    }
    struct sockaddr_in servaddr;
    for (p = si; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            printf("Failed to creat socket.\n");
            continue;
        }
        printf("Socket created\n");
        break;
    }
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv));

    if (p == NULL)
    {
        printf("Failed to create socket, p=NULL.\n");
        freeaddrinfo(si);
        exit(0);
    }

    ssize_t sentbytes;

    socklen_t addr_len = sizeof(servaddr);
    int bytes = -1;
    int tries = 0;
    char recvBuf[256];
    char sendBuf[5];
    bool isRunning = true;
    fd_set currentSockets;
    fd_set readySockets;
    FD_ZERO(&currentSockets);
    FD_ZERO(&readySockets);
    FD_SET(sockfd, &currentSockets);
    FD_SET(STDIN_FILENO, &currentSockets);
    int fdMax = sockfd;
    int nfds = 0;
    signal(SIGINT, intSignal);
    while (tries < 3 && bytes < 0)
    {
        memset(recvBuf, 0, sizeof(recvBuf));
        if ((sentbytes = sendto(sockfd, PROTOCOL, sizeof(PROTOCOL), 0, p->ai_addr, p->ai_addrlen)) == -1)
        {
            printf("Failed to send via sento function. \n");
            exit(0);
        }
        bytes = recvfrom(sockfd, recvBuf, sizeof(recvBuf), 0, (struct sockaddr *)&servaddr, &addr_len);
        if (bytes == -1 && tries < 2)
        {
            printf("Server did not respond... Trying again.\n");
        }
        tries++;
    }
    if (bytes == -1)
    {
        printf("Failed to recive message or server did not respond. Exiting...\n");
        exit(0);
    }
    else
    {
        if (strstr(recvBuf, "ERROR") != nullptr)
        {
            printf("%s", recvBuf);
            exit(0);
        }
        else
        {
            printf("Protocol accepted. Protocol: %s%s", PROTOCOL, recvBuf);
        }

        while (isRunning)
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
                memset(sendBuf, 0, sizeof(sendBuf));
                std::cin.getline(sendBuf, sizeof(sendBuf));
                std::cin.clear();
                if (strcmp(sendBuf, "0") == 0)
                {
                    isRunning = false;
                }
                else
                {
                    sendto(sockfd, sendBuf, sizeof(sendBuf), 0, p->ai_addr, p->ai_addrlen);
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
                else
                {
                    printf("%s", recvBuf);
                }
            }
        }
    }
    close(sockfd);
    return 0;
}
