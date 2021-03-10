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

#define MENU "Please select:\n1. Play\n2. Watch\n3. Scoreboard\n0. Quit\n"
#define PROTOCOL "RPS UDP 1.0\n"

struct cli
{
  struct sockaddr_in adress;
  struct timeval tid;
  bool isInGame;
  bool isReady;
  bool inQueue;
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
  addrinfo sa, *si, *p;
  memset(&sa, 0, sizeof(sa));
  sa.ai_family = AF_UNSPEC;
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
  struct sockaddr_in client;
  socklen_t client_len;
  for (p = si; p != NULL; p = p->ai_next)
  {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
    {
      printf("Failed to creat socket.\n");
      continue;
    }
    if ((bind(sockfd, p->ai_addr, p->ai_addrlen)) != 0)
    {
      printf("Failed to bind.\n");
      close(sockfd);
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

  struct itimerval alarmTime;

  alarmTime.it_interval.tv_sec = 2;
  alarmTime.it_interval.tv_usec = 2;
  alarmTime.it_value.tv_sec = 2;
  alarmTime.it_value.tv_usec = 2;

  signal(SIGALRM, checkJobbList);
  setitimer(ITIMER_REAL, &alarmTime, NULL);
  int bytes = -1;
  client_len = sizeof(client);
  int clientNr = 0;
  int currentClient = -1;
  fd_set currentSockets;
  fd_set readySockets;
  FD_ZERO(&currentSockets);
  FD_ZERO(&readySockets);
  FD_SET(sockfd, &currentSockets);
  char recvBuf[256];
  char sendBuf[256];
  bool clientFound = false;
  while (true)
  {
    bytes = recvfrom(sockfd, recvBuf, sizeof(recvBuf), 0, (struct sockaddr *)&client, &client_len);
    if (bytes < 0)
    {
      continue;
    }
    else
    {
      //check if client already exist, else check if protocol recived
      clientFound = false;
      currentClient = -1;
      for (size_t i = 0; i < clients.size() && !clientFound; i++)
      {
        if (clients.at(i)->adress.sin_addr.s_addr == client.sin_addr.s_addr && clients.at(i)->adress.sin_port == client.sin_port)
        {
          clientFound = true;
          currentClient = i;
        }
      }
      if (clientFound == false)
      {
        if (strcmp(recvBuf, PROTOCOL) >= 0)
        {
          struct cli newClient;
          newClient.adress = client;
          gettimeofday(&newClient.tid, NULL);
          newClient.isInGame = false;
          newClient.isReady = false;
          newClient.inQueue = false;
          clients.push_back(&newClient);
          printf("Client added\n");
          sendto(sockfd, MENU, sizeof(MENU), 0, (struct sockaddr *)&client, client_len);
        }
        else
        {
          sendto(sockfd, "ERROR Wrong protocol\n", sizeof("ERROR Wrong protocol\n"), 0, (struct sockaddr *)&client, client_len);
        }
      }
      else
      {
        //check all messages and what's going on
        if (clients.at(currentClient)->isInGame == false && clients.at(currentClient)->isReady == false && clients.at(currentClient)->inQueue == false)
        {
          if (strcmp(recvBuf, "1") == 0)
          {
            clients.at(currentClient)->inQueue = true;
            queues.push_back(clients.at(currentClient));

            if (queues.size() > 1)
            {
              for (size_t i = 0; i < 2; i++)
              {
                queues.at(i)->isInGame = true;
              }
              queues.erase(queues.begin(), queues.begin() + 2);
              printf("Queue erased\n");
            }
          }
        }
        if (clients.at(currentClient)->isInGame == true && clients.at(currentClient)->isReady == false)
        {
          if (strcmp(recvBuf, "\n") == 0)
          {
            clients.at(currentClient)->isReady = true;
          }
        }
        for (size_t i; i < clients.size(); i++)
        {
          printf("i=%d\n",(int)i);
          if (clients.at(i)->inQueue == true && clients.at(i)->isInGame == false && clients.at(i)->isReady == false)
          {
            sendto(sockfd, "You are in queue, waiting for another client to connect.\n", sizeof("You are in queue, waiting for another client to connect.\n"), 0, (struct sockaddr *)&client, client_len);
          }
          else if (clients.at(i)->isInGame == true && clients.at(i)->isReady == false && clients.at(i)->inQueue == false)
          {
            sendto(sockfd, "Game is ready. Press Enter to confirm that you are ready.\n", sizeof("Game is ready. Press Enter to confirm that you are ready.\n"), 0, (struct sockaddr *)&client, client_len);
          }
        }
      }
    }
  }

  close(sockfd);
  return (0);
}
