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
#define DEBUG

void intSignal(int sig)
{
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
    msg.type = htons(22);
    msg.message = htonl(0);
    msg.protocol = htons(17);
    msg.major_version = htons(1);
    msg.minor_version = htons(0);

    printf("Message sent.\n");

    socklen_t addr_len = sizeof(servaddr);
    int bytes = -1;
    int tries = 0;
    signal(SIGINT, intSignal);
    while (tries < 3 && bytes < 0)
    {
        if ((sentbytes = sendto(sockfd, &msg, sizeof(msg), 0, p->ai_addr, p->ai_addrlen)) == -1)
        {
            printf("Failed to send via sento function. \n");
            exit(0);
        }
        bytes = recvfrom(sockfd, &protmsg, sizeof(protmsg), 0, (struct sockaddr *)&servaddr, &addr_len);
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
        sleep(3);
    }
    close(sockfd);
    return 0;
}
