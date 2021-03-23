#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <vector>
#include <errno.h>

#define MENU "Please select:\n1. Play\n2. Watch\n3. Scoreboard\n0. Quit\n"
#define PROTOCOL "RPS TCP 1.0\n"

struct cli
{
  struct timeval tid;
  bool isInGame;
  bool isReady;
  bool inQueue;
  int sockID;
};
std::vector<cli *> clients;
std::vector<cli *> queues;
void checkJobbList(int signum)
{
  struct timeval t;
  gettimeofday(&t, NULL);
  for (size_t i = 0; i < clients.size(); i++)
  {
    if (t.tv_sec - clients.at(i)->tid.tv_sec > 10)
    {
      /*  clients.erase(clients.begin() + i);
      printf("Client slept...\n");
      */
    }
  }
  return;
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
  int sockfd, len, connfd;
  struct sockaddr_in client;

  addrinfo sa, *si, *p;
  memset(&sa, 0, sizeof(sa));
  sa.ai_family = AF_UNSPEC;
  sa.ai_socktype = SOCK_STREAM;
  sa.ai_flags = AI_PASSIVE;
  struct timeval tv;
  tv.tv_sec = 5;
  tv.tv_usec = 0;
  if (int rv = getaddrinfo(Desthost, Destport, &sa, &si) != 0)
  {
    fprintf(stderr, "%s\n", gai_strerror(rv));
    exit(0);
  }

  for (p = si; p != NULL; p = p->ai_next)
  {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
    {
      continue;
    }
    if ((bind(sockfd, p->ai_addr, p->ai_addrlen)) != 0)
    {
      printf("Failed to bind.\n");
      close(sockfd);
      continue;
    }
    break;
  }
  if (p == NULL)
  {
    printf("Couldn't create/bind socket.\n");
    exit(0);
  }
  //setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv));
  freeaddrinfo(si);

  if (listen(sockfd, 5) != 0)
  {
    printf("Failed to listen.\n");
    exit(0);
  }
  len = sizeof(client);
  char recvBuf[256];
  char sendBuf[256];
  fd_set currentSockets;
  fd_set readySockets;
  FD_ZERO(&currentSockets);
  FD_ZERO(&readySockets);
  FD_SET(sockfd, &currentSockets);
  int fdMax = sockfd;
  int nfds = 0;
  int reciver;
  int cC = -1; //cC=currentClient
  //signal(SIGINT, intSignal);
  while (true)
  {
    readySockets = currentSockets;

    nfds = select(fdMax + 1, &readySockets, NULL, NULL, NULL);
    if (nfds == -1)
    {
      printf("Something went wrong with select.\n");
      printf("%s\n", strerror(errno));
      break;
    }
    for (int i = sockfd; i < fdMax + 1; i++)
    {
      if (FD_ISSET(i, &readySockets))
      {
        if (i == sockfd)
        {
          if ((connfd = accept(sockfd, (struct sockaddr *)&client, (socklen_t *)&len)) == -1)
          {
            continue;
          }
          else
          {
            struct cli newClient;
            newClient.sockID = connfd;
            newClient.inQueue = false;
            newClient.isInGame = false;
            newClient.isReady = false;
            gettimeofday(&newClient.tid, NULL);
            FD_SET(newClient.sockID, &currentSockets);
            clients.push_back(&newClient);
            char buf[sizeof(PROTOCOL)] = PROTOCOL;
            send(connfd, buf, strlen(buf), 0);
            printf("Server protocol %s", buf);
            if (newClient.sockID > fdMax)
            {
              fdMax = newClient.sockID;
            }
          }
        }
        else
        {
          if (clients.size() > 0)
          {
            memset(recvBuf, 0, sizeof(recvBuf));
            reciver = recv(i, recvBuf, sizeof(recvBuf), 0);
            printf("Rcv:%s\n\n", recvBuf);
            if (reciver <= 0)
            {
              close(i);
              for (size_t j = 0; j < clients.size(); j++)
              {
                if (i == clients[j]->sockID)
                {
                  clients.erase(clients.begin() + j);
                  FD_CLR(i, &currentSockets);
                  break;
                }
              }
              continue;
            }
            else if (strstr(recvBuf, "OK\n") != nullptr)
            {
              send(i, MENU, strlen(MENU), 0);
            }
            else if (strcmp(recvBuf, "1") == 0)
            {
              cC = -1;
              for (size_t j = 0; j < clients.size() && cC == -1; j++)
              {
                if (clients.at(j)->sockID == i)
                {
                  cC = j;
                }
              }
              if (cC > -1 && !clients.at(cC)->inQueue && !clients.at(cC)->isReady && !clients.at(cC)->isInGame)
              {
                clients.at(cC)->inQueue = true;
                queues.push_back(clients.at(cC));
                printf("In queue\n");
                if (queues.size() < 2)
                {
                  send(i, "In queue, need another player to start game.\nPress 9 to leave queue.\n", strlen("In queue, need another player to start game.\nPress 9 to leave queue.\n"), 0);
                }
                else if (queues.size() >= 2)
                {
                  for (int j = 0; j < 2; j++)
                  {
                    queues.at(0)->inQueue = false;
                    queues.at(0)->isInGame = true;
                    send(queues.at(0)->sockID, "A game is ready, press r to be ready\n", strlen("A game is ready, press r to be ready\n"), 0);
                    queues.erase(queues.begin());
                  }
                }
              }
            }
            else if (strcmp(recvBuf, "9") == 0)
            {
              cC = -1;
              for (size_t j = 0; j < queues.size() && cC == -1; j++)
              {
                if (queues.at(j)->sockID == i)
                {
                  cC = j;
                }
              }
              if (cC > -1 && queues.at(cC)->inQueue && !queues.at(cC)->isReady && !queues.at(cC)->isInGame)
              {
                queues.at(cC)->inQueue = false;
                queues.erase((queues.begin() + cC));
                send(i, MENU, strlen(MENU), 0);
              }
            }
            else
            {
              send(i, "ERROR Wrong format on the message sent.\n", strlen("ERROR Wrong format on the message sent.\n"), 0);
            }
          }
        }
      }
      FD_CLR(i, &readySockets);
    }
  }
  close(sockfd);
  return 0;
}
