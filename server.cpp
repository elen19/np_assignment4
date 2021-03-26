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
#define OPTIONS "Please select an option!\n1.Rock\n2.Paper\n3.Scissor\n"
#define PROTOCOL "RPS TCP 1.0\n"

struct cli
{
  struct timeval tid;
  bool isInGame;
  bool isReady;
  bool inQueue;
  int sockID;
};
struct game
{
  struct cli *player1;
  struct cli *player2;
  int p1score, p2score, winner, p1Option, p2Option;
};
std::vector<cli> clients;
std::vector<game *> games;
std::vector<cli *> queues;
void checkJobbList(int signum)
{

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
  struct timeval t;
  struct timeval gameTime;
  t.tv_sec = 0;
  t.tv_usec = 1000000;
  gameTime.tv_sec = 0;
  gameTime.tv_usec = 0;
  int cC = -1; //cC=currentClient
  //signal(SIGINT, intSignal);
  while (true)
  {
    readySockets = currentSockets;

    for (size_t i = 0; i < games.size(); i++)
    {
      if (games.at(i)->p1Option != 0 && games.at(i)->p2Option != 0)
      {
        if (games.at(i)->p1Option == 1 && games.at(i)->p2Option == 2)
        {
          games.at(i)->p2score++;
        }
        else if (games.at(i)->p1Option == 1 && games.at(i)->p2Option == 3)
        {
          games.at(i)->p1score++;
        }
        else if (games.at(i)->p1Option == 2 && games.at(i)->p2Option == 1)
        {
          games.at(i)->p1score++;
        }
        else if (games.at(i)->p1Option == 2 && games.at(i)->p2Option == 3)
        {
          games.at(i)->p2score++;
        }
        else if (games.at(i)->p1Option == 3 && games.at(i)->p2Option == 1)
        {
          games.at(i)->p2score++;
        }
        else if (games.at(i)->p1Option == 3 && games.at(i)->p2Option == 2)
        {
          games.at(i)->p1score++;
        }
        games.at(i)->p1Option = 0;
        games.at(i)->p2Option = 0;
        printf("Score %d - %d\n", games.at(i)->p1score, games.at(i)->p2score);
        sscanf(sendBuf, "Score %d - %d\n", games.at(i)->p1score, games.at(i)->p2score);
        printf("%s",sendBuf);
        send(games.at(i)->player1->sockID, sendBuf, strlen(sendBuf), 0);
        send(games.at(i)->player2->sockID, sendBuf, strlen(sendBuf), 0);
        if (games.at(i)->p1score < 3 && games.at(i)->p2score < 3)
        {
          send(games.at(i)->player1->sockID, OPTIONS, strlen(OPTIONS), 0);
          send(games.at(i)->player2->sockID, OPTIONS, strlen(OPTIONS), 0);
        }
      }
      /*gettimeofday(&gameTime, NULL);
      if ((gameTime.tv_usec - games.at(i)->player1->tid.tv_usec) > 200000)
      {
        send(games.at(i)->player1->sockID, "Hejhej\n", strlen("Hejhej\n"), 0);
      }
      if ((gameTime.tv_usec - games.at(i)->player2->tid.tv_usec) > 200000)
      {
        send(games.at(i)->player2->sockID, "Hejhej\n", strlen("Hejhej\n"), 0);
      }*/
    }

    nfds = select(fdMax + 1, &readySockets, NULL, NULL, &t);
    if (nfds == -1)
    {

      printf("Something went wrong with select.\n");
      printf("%s\n", strerror(errno));
      break;
    }
    if (nfds == 0)
    {
      t.tv_usec += 100000;
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
            clients.push_back(newClient);
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
          memset(recvBuf, 0, sizeof(recvBuf));
          reciver = recv(i, recvBuf, sizeof(recvBuf), 0);
          if (clients.size() > 0)
          {
            if (reciver > 0)
            {
              for (size_t j = 0; j < games.size(); j++)
              {
                if (games.at(j)->player1->sockID == i)
                {
                  if (games.at(j)->p1Option == 0 && (strcmp(recvBuf, "1") == 0 || strcmp(recvBuf, "2") == 0 || strcmp(recvBuf, "3") == 0))
                  {
                    if (strcmp(recvBuf, "1") == 0)
                    {
                      games.at(j)->p1Option = 1;
                    }
                    else if (strcmp(recvBuf, "2") == 0)
                    {
                      games.at(j)->p1Option = 2;
                    }
                    else if (strcmp(recvBuf, "3") == 0)
                    {
                      games.at(j)->p1Option = 3;
                    }
                    gettimeofday(&games.at(j)->player1->tid, NULL);
                  }
                }
                else if (games.at(j)->player2->sockID == i)
                {
                  if (games.at(j)->p2Option == 0 && (strcmp(recvBuf, "1") == 0 || strcmp(recvBuf, "2") == 0 || strcmp(recvBuf, "3") == 0))
                  {
                    if (strcmp(recvBuf, "1") == 0)
                    {
                      games.at(j)->p2Option = 1;
                    }
                    else if (strcmp(recvBuf, "2") == 0)
                    {
                      games.at(j)->p2Option = 2;
                    }
                    else if (strcmp(recvBuf, "3") == 0)
                    {
                      games.at(j)->p2Option = 3;
                    }
                    gettimeofday(&games.at(j)->player2->tid, NULL);
                  }
                }
              }
            }
            if (reciver <= 0)
            {
              close(i);
              for (size_t j = 0; j < clients.size(); j++)
              {
                if (i == clients[j].sockID)
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
                if (clients.at(j).sockID == i)
                {
                  cC = j;
                }
              }
              if (cC > -1 && !clients.at(cC).inQueue && !clients.at(cC).isReady && !clients.at(cC).isInGame)
              {
                clients.at(cC).inQueue = true;
                queues.push_back(&clients.at(cC));
                if (queues.size() < 2)
                {
                  send(i, "In queue, need another player to start game.\nPress 9 to leave queue.\n", strlen("In queue, need another player to start game.\nPress 9 to leave queue.\n"), 0);
                }
                else if (queues.size() >= 2)
                {
                  struct game newGame;
                  newGame.p1score = 0;
                  newGame.p2score = 0;
                  newGame.winner = -1;
                  newGame.player1 = queues.at(0);
                  newGame.player2 = queues.at(1);
                  newGame.p1Option = 0;
                  newGame.p2Option = 0;
                  games.push_back(&newGame);
                  for (int j = 0; j < 2; j++)
                  {
                    queues.at(j)->inQueue = false;
                    queues.at(j)->isInGame = true;
                    send(queues.at(j)->sockID, "A game is ready, press r to be ready\n", strlen("A game is ready, press r to be ready\n"), 0);
                  }
                  for (int j = 0; j < 2; j++)
                  {
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
            else if (strcmp(recvBuf, "r") == 0)
            {
              cC = -1;
              for (size_t j = 0; j < clients.size() && cC == -1; j++)
              {
                if (clients.at(j).sockID == i)
                {
                  cC = j;
                }
              }
              if (cC > -1 && !clients.at(cC).inQueue && !clients.at(cC).isReady && clients.at(cC).isInGame)
              {
                clients.at(cC).isReady = true;
                send(i, "Waiting for other player to be ready.\n", strlen("Waiting for other player to be ready.\n"), 0);
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
