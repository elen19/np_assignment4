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

int terminate = 0;
struct cli
{
  struct sockaddr_in adress;
  struct timeval tid;
};
std::vector<cli> clients;
void checkJobbList(int signum)
{
  struct timeval t;
  printf("Let me be, I want to sleep.\n");
  gettimeofday(&t, NULL);
  for (size_t i = 0; i < clients.size(); i++)
  {
    if (t.tv_sec - clients.at(i).tid.tv_sec > 10)
    {
      clients.erase(clients.begin() + i);
      printf("Client slept...\n");
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

  /* 
     Prepare to setup a reoccurring event every 10s. If it_interval, or it_value is omitted, it will be a single alarm 10s after it has been set. 
  */
  struct itimerval alarmTime;

  alarmTime.it_interval.tv_sec = 2;
  alarmTime.it_interval.tv_usec = 2;
  alarmTime.it_value.tv_sec = 2;
  alarmTime.it_value.tv_usec = 2;

  /* Regiter a callback function, associated with the SIGALRM signal, which will be raised when the alarm goes of */
  signal(SIGALRM, checkJobbList);
  setitimer(ITIMER_REAL, &alarmTime, NULL); // Start/register the alarm.
  int bytes = -1;
  client_len = sizeof(client);
  int clientNr = 0;
  int currentClient = -1;
  while (terminate == 0)
  {
  

  }
  close(sockfd);
  return (0);
}
