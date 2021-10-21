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
	bool spectating;
	int sockID;
	int answer;
	size_t gameID;
	int score;
	size_t totalTime;
};
struct game
{
	struct cli * player1;
	struct cli * player2;
	int p1score, p2score, winner, p1Option, p2Option;
	struct timeval tid;
};
std::vector<cli> clients;
std::vector<game*> games;
std::vector<cli*> queues;
std::vector<size_t> timeScore;
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
	int reciver = 0;
	struct timeval t;
	struct timeval gameTime;
	struct timeval timeChecker;
	t.tv_sec = 0;
	t.tv_usec = 0;
	gameTime.tv_sec = 0;
	gameTime.tv_usec = 0;
	timeChecker.tv_usec = 0;
	timeChecker.tv_sec = 0;
	int cC = -1;	//cC=currentClient
	//signal(SIGINT, intSignal);
	while (true)
	{
		gettimeofday(&gameTime, NULL);
		gettimeofday(&timeChecker, NULL);
		readySockets = currentSockets;
		if (games.size() > 0)
		{
			cC = -1;
			for (size_t i = 0; i < games.size(); i++)
			{
				size_t number = (gameTime.tv_sec - games.at(i)->tid.tv_sec);
				for (size_t j = 0; j < clients.size(); j++)
				{
					for (size_t g = 0; g < clients.size(); g++)
					{
						if (clients.at(j).sockID != clients.at(g).sockID && clients.at(j).isInGame && clients.at(g).isInGame)
						{
							if (clients.at(j).gameID == i && clients.at(g).gameID == i)
							{
								cC = j;
								if (clients.at(j).answer != 0 && clients.at(g).answer != 0)
								{
									for (size_t k = 0; k < clients.size(); k++)
									{
										if (clients.at(k).spectating && clients.at(k).gameID == i)
										{
											if (clients.at(j).answer == 1 && clients.at(g).answer == 2)
											{
												send(clients.at(k).sockID, "Player 1 selected: Rock\tPlayer 2 selected: Paper.\n", strlen("Player 1 selected: Rock\tPlayer 2 selected: Paper.\n"), 0);
											}
											else if (clients.at(j).answer == 1 && clients.at(g).answer == 3)
											{
												send(clients.at(k).sockID, "Player 1 selected: Rock\tPlayer 2 selected: Scissor.\n", strlen("Player 1 selected: Rock\tPlayer 2 selected: Scissor.\n"), 0);
											}
											else if (clients.at(j).answer == 1 && clients.at(g).answer == 1)
											{
												send(clients.at(k).sockID, "Player 1 selected: Rock\tPlayer 2 selected: Rock.\n", strlen("Player 1 selected: Rock\tPlayer 2 selected: Rock.\n"), 0);
											}
											else if (clients.at(j).answer == 2 && clients.at(g).answer == 1)
											{
												send(clients.at(k).sockID, "Player 1 selected: Paper\tPlayer 2 selected: Rock.\n", strlen("Player 1 selected: Paper\tPlayer 2 selected: Rock.\n"), 0);
											}
											else if (clients.at(j).answer == 2 && clients.at(g).answer == 2)
											{
												send(clients.at(k).sockID, "Player 1 selected: Paper\tPlayer 2 selected: Paper.\n", strlen("Player 1 selected: Paper\tPlayer 2 selected: Paper.\n"), 0);
											}
											else if (clients.at(j).answer == 2 && clients.at(g).answer == 3)
											{
												send(clients.at(k).sockID, "Player 1 selected: Paper\tPlayer 2 selected: Scissor.\n", strlen("Player 1 selected: Paper\tPlayer 2 selected: Scissor.\n"), 0);
											}
											else if (clients.at(j).answer == 3 && clients.at(g).answer == 1)
											{
												send(clients.at(k).sockID, "Player 1 selected: Scissor\tPlayer 2 selected: Rock.\n", strlen("Player 1 selected: Scissor\tPlayer 2 selected: Rock.\n"), 0);
											}
											else if (clients.at(j).answer == 3 && clients.at(g).answer == 2)
											{
												send(clients.at(k).sockID, "Player 1 selected: Scissor\tPlayer 2 selected: Paper.\n", strlen("Player 1 selected: Scissor\tPlayer 2 selected: Paper.\n"), 0);
											}
											else if (clients.at(j).answer == 3 && clients.at(g).answer == 3)
											{
												send(clients.at(k).sockID, "Player 1 selected: Scissor\tPlayer 2 selected: Scissor.\n", strlen("Player 1 selected: Scissor\tPlayer 2 selected: Scissor.\n"), 0);
											}
										}
									}
									if (clients.at(j).answer == 1 && clients.at(g).answer == 2)
									{
										clients.at(g).score++;
									}
									else if (clients.at(j).answer == 1 && clients.at(g).answer == 3)
									{
										clients.at(j).score++;
									}
									else if (clients.at(j).answer == 2 && clients.at(g).answer == 1)
									{
										clients.at(j).score++;
									}
									else if (clients.at(j).answer == 2 && clients.at(g).answer == 3)
									{
										clients.at(g).score++;
									}
									else if (clients.at(j).answer == 3 && clients.at(g).answer == 1)
									{
										clients.at(g).score++;
									}
									else if (clients.at(j).answer == 3 && clients.at(g).answer == 2)
									{
										clients.at(j).score++;
									}
									clients.at(j).answer = 0;
									clients.at(g).answer = 0;
									memset(sendBuf, 0, sizeof(sendBuf));
									sprintf(sendBuf, "Score %d - %d\n", clients.at(j).score, clients.at(g).score);
									printf("%s", sendBuf);
									send(clients.at(j).sockID, sendBuf, strlen(sendBuf), 0);
									memset(sendBuf, 0, sizeof(sendBuf));
									sprintf(sendBuf, "Score %d - %d\n", clients.at(g).score, clients.at(j).score);
									send(clients.at(g).sockID, sendBuf, strlen(sendBuf), 0);
									memset(sendBuf, 0, sizeof(sendBuf));
									sprintf(sendBuf, "Score %d - %d\n", clients.at(j).score, clients.at(g).score);
									clients.at(j).totalTime = clients.at(j).totalTime + (gameTime.tv_sec - games.at(i)->tid.tv_sec);
									clients.at(g).totalTime = clients.at(g).totalTime + (gameTime.tv_sec - games.at(i)->tid.tv_sec);
									for (size_t k = 0; k < clients.size(); k++)
									{
										if (clients.at(k).spectating && clients.at(k).gameID == i)
										{
											send(clients.at(k).sockID, sendBuf, strlen(sendBuf), 0);
										}
									}
									if (clients.at(j).score < 3 && clients.at(g).score < 3)
									{
										gettimeofday(&games.at(i)->tid, NULL);
									}
									else
									{
										if (clients.at(j).score == 3)
										{
											int rounds = 0;
											size_t averageTime = 0;
											rounds = (clients.at(j).score + clients.at(g).score);
											averageTime = (clients.at(j).totalTime / rounds);
											send(clients.at(j).sockID, "You won!\n", strlen("You won!\n"), 0);
											send(clients.at(g).sockID, "You lost!\n", strlen("You lost!\n"), 0);
											timeScore.push_back(averageTime);
										}
										else if (clients.at(g).score == 3)
										{
											int rounds = 0;
											size_t averageTime = 0;
											rounds = (clients.at(j).score + clients.at(g).score);
											averageTime = (clients.at(g).totalTime / rounds);
											send(clients.at(g).sockID, "You won!\n", strlen("You won!\n"), 0);
											send(clients.at(j).sockID, "You lost!\n", strlen("You lost!\n"), 0);
											timeScore.push_back(averageTime);
										}
										else
										{
											break;
										}
										for (size_t k = 0; k < clients.size(); k++)
										{
											if (clients.at(k).spectating && clients.at(k).gameID == clients.at(j).gameID)
											{
												send(clients.at(k).sockID, "The game is now over, sending you back to start menu.\n", strlen("The game is now over, sending you back to start menu.\n"), 0);
												send(clients.at(k).sockID, MENU, strlen(MENU), 0);
												clients.at(k).spectating = false;
												clients.at(k).gameID = -1;
											}
										}
										clients.at(j).totalTime = 0;
										clients.at(g).totalTime = 0;
										clients.at(j).gameID = -1;
										clients.at(g).gameID = -1;
										clients.at(j).isInGame = false;
										clients.at(g).isInGame = false;
										clients.at(j).isReady = false;
										clients.at(g).isReady = false;
										clients.at(j).inQueue = false;
										clients.at(g).inQueue = false;
										clients.at(j).answer = 0;
										clients.at(g).answer = 0;
										clients.at(j).score = 0;
										clients.at(g).score = 0;
										send(clients.at(j).sockID, MENU, strlen(MENU), 0);
										send(clients.at(g).sockID, MENU, strlen(MENU), 0);
										games.erase(games.begin() + i);
										for (size_t p = 0; p < clients.size(); p++)
										{
											if (clients.at(p).gameID > i)
											{
												clients.at(p).gameID--;
											}
										}
									}
								}
								else if ((gameTime.tv_sec - clients.at(j).tid.tv_sec) == 3 && (clients.at(g).answer == 1 || clients.at(g).answer == 2 || clients.at(g).answer == 3))
								{
									clients.at(g).score++;
									clients.at(j).answer = 0;
									clients.at(g).answer = 0;
									memset(sendBuf, 0, sizeof(sendBuf));
									sprintf(sendBuf, "You were too slow to choose. You lost the round.\nScore %d - %d\n", clients.at(g).score, clients.at(j).score);
									send(clients.at(j).sockID, sendBuf, strlen(sendBuf), 0);
									memset(sendBuf, 0, sizeof(sendBuf));
									sprintf(sendBuf, "The other player were to slow to choose. You won the round.\nScore %d - %d\n", clients.at(g).score, clients.at(j).score);
									send(clients.at(g).sockID, sendBuf, strlen(sendBuf), 0);
									clients.at(j).totalTime = clients.at(j).totalTime + (gameTime.tv_sec - games.at(i)->tid.tv_sec);
									clients.at(g).totalTime = clients.at(g).totalTime + (gameTime.tv_sec - games.at(i)->tid.tv_sec);
									for (size_t k = 0; k < clients.size(); k++)
									{
										if (clients.at(k).spectating && clients.at(k).gameID == i)
										{
											send(clients.at(k).sockID, sendBuf, strlen(sendBuf), 0);
										}
									}
									gettimeofday(&games.at(i)->tid, NULL);
									gettimeofday(&clients.at(j).tid, NULL);
									gettimeofday(&clients.at(g).tid, NULL);
									if (clients.at(j).score == 3)
									{
										int rounds = 0;
										size_t averageTime = 0;
										rounds = (clients.at(j).score + clients.at(g).score);
										averageTime = (clients.at(j).totalTime / rounds);
										send(clients.at(j).sockID, "You won!\n", strlen("You won!\n"), 0);
										send(clients.at(g).sockID, "You lost!\n", strlen("You lost!\n"), 0);
										timeScore.push_back(averageTime);
									}
									else if (clients.at(g).score == 3)
									{
										int rounds = 0;
										size_t averageTime = 0;
										rounds = (clients.at(j).score + clients.at(g).score);
										averageTime = (clients.at(j).totalTime / rounds);
										send(clients.at(g).sockID, "You won!\n", strlen("You won!\n"), 0);
										send(clients.at(j).sockID, "You lost!\n", strlen("You lost!\n"), 0);
										timeScore.push_back(averageTime);
									}
									else
									{
										break;
									}
									for (size_t k = 0; k < clients.size(); k++)
									{
										if (clients.at(k).spectating && clients.at(k).gameID == clients.at(j).gameID)
										{
											send(clients.at(k).sockID, "The game is now over, sending you back to start menu.\n", strlen("The game is now over, sending you back to start menu.\n"), 0);
											send(clients.at(k).sockID, MENU, strlen(MENU), 0);
											clients.at(k).spectating = false;
											clients.at(k).gameID = -1;
										}
									}
									clients.at(j).totalTime = 0;
									clients.at(g).totalTime = 0;
									clients.at(j).gameID = -1;
									clients.at(g).gameID = -1;
									clients.at(j).isInGame = false;
									clients.at(g).isInGame = false;
									clients.at(j).isReady = false;
									clients.at(g).isReady = false;
									clients.at(j).inQueue = false;
									clients.at(g).inQueue = false;
									clients.at(j).answer = 0;
									clients.at(g).answer = 0;
									clients.at(j).score = 0;
									clients.at(g).score = 0;
									send(clients.at(j).sockID, MENU, strlen(MENU), 0);
									send(clients.at(g).sockID, MENU, strlen(MENU), 0);

									games.erase(games.begin() + i);
									for (size_t p = 0; p < clients.size(); p++)
									{
										if (clients.at(p).gameID > i)
										{
											clients.at(p).gameID--;
										}
									}
								}
								else if (((gameTime.tv_sec - clients.at(j).tid.tv_sec) == 3) && clients.at(j).answer == 0 && clients.at(g).answer == 0)
								{
									memset(sendBuf, 0, strlen(sendBuf));
									sprintf(sendBuf, "You were too slow to choose. You lost the round.\nScore %d - %d\n", clients.at(j).score, clients.at(g).score);
									send(clients.at(j).sockID, sendBuf, strlen(sendBuf), 0);
									memset(sendBuf, 0, strlen(sendBuf));
									sprintf(sendBuf, "You were too slow to choose. You lost the round.\nScore %d - %d\n", clients.at(g).score, clients.at(j).score);
									send(clients.at(g).sockID, sendBuf, strlen(sendBuf), 0);
									gettimeofday(&games.at(i)->tid, NULL);
									gettimeofday(&clients.at(j).tid, NULL);
									gettimeofday(&clients.at(g).tid, NULL);
								}
							}
						}
						if (clients.at(j).gameID == i && clients.at(g).gameID == i && clients.at(j).sockID != clients.at(g).sockID && clients.at(j).isInGame && clients.at(g).isInGame && ((gameTime.tv_sec - clients.at(j).tid.tv_sec) == 1))
						{
							if (number >= 0 && number < 5)
							{
								gettimeofday(&clients.at(j).tid, NULL);
								gettimeofday(&clients.at(g).tid, NULL);
								if (number == 0)
								{
									memset(sendBuf, 0, sizeof(sendBuf));
									sprintf(sendBuf, "Both players are ready!\n");
								}
								else if (number == 1)
								{
									memset(sendBuf, 0, sizeof(sendBuf));
									sprintf(sendBuf, "Game starts in 3 seconds!\n");
								}
								else if (number == 2)
								{
									memset(sendBuf, 0, sizeof(sendBuf));
									sprintf(sendBuf, "Game starts in 2 seconds!\n");
								}
								else if (number == 3)
								{
									memset(sendBuf, 0, sizeof(sendBuf));
									sprintf(sendBuf, "Game starts in 1 seconds!\n");
								}
								else if (number == 4)
								{
									memset(sendBuf, 0, sizeof(sendBuf));
									sprintf(sendBuf, OPTIONS);
								}
								if (clients.at(j).isInGame && !clients.at(j).spectating && !clients.at(g).spectating)
								{
									send(clients.at(j).sockID, sendBuf, strlen(sendBuf), 0);
									send(clients.at(g).sockID, sendBuf, strlen(sendBuf), 0);
									for (size_t k = 0; k < clients.size(); k++)
									{
										if (clients.at(k).spectating && clients.at(k).gameID == i && number != 4)
										{
											send(clients.at(k).sockID, sendBuf, strlen(sendBuf), 0);
										}
									}
								}
							}
						}
					}
				}
			}
		}
		nfds = select(fdMax + 1, &readySockets, NULL, NULL, &t);
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
					if ((connfd = accept(sockfd, (struct sockaddr *) &client, (socklen_t*) &len)) == -1)
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
						newClient.spectating = false;
						newClient.answer = 0;
						newClient.gameID = -1;
						newClient.score = 0;
						newClient.totalTime = 0;
						gettimeofday(&newClient.tid, NULL);
						FD_SET(newClient.sockID, &currentSockets);
						clients.push_back(newClient);
						char buff[sizeof(PROTOCOL)] = PROTOCOL;
						send(connfd, buff, strlen(buff), 0);
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
							if (strstr(recvBuf, "OK") != nullptr)
							{
								cC = -1;
								for (size_t j = 0; j < clients.size() && cC == -1; j++)
								{
									if (clients.at(j).sockID == i)
									{
										cC = j;
									}
								}
								send(clients.at(cC).sockID, MENU, strlen(MENU), 0);
								break;
							}
							else if (strstr(recvBuf, "1") != nullptr)
							{
								cC = -1;
								for (size_t j = 0; j < clients.size() && cC == -1; j++)
								{
									if (clients.at(j).sockID == i)
									{
										cC = j;
									}
								}
								if (cC > -1 && !clients.at(cC).inQueue && !clients.at(cC).isReady && !clients.at(cC).isInGame && !clients.at(cC).spectating && queues.size() < 2)
								{
									clients.at(cC).inQueue = true;
									queues.push_back(&clients.at(cC));
									if (queues.size() < 2)
									{
										send(i, "In queue, need another player to start game.\nPress 9 to leave queue.\n", strlen("In queue, need another player to start game.\nPress 9 to leave queue.\n"), 0);
									}
									else if (queues.size() >= 2)
									{
										if (queues.at(0)->sockID == i || queues.at(1)->sockID == i)
										{
											for (size_t j = 0; j < queues.size(); j++)
											{
												send(queues.at(j)->sockID, "The game is ready to start, press 'r' to accept.\n", strlen("The game is ready to start, press 'r' to accept.\n"), 0);
											}
										}
									}
								}
							}
							else if (strstr(recvBuf, "2") != nullptr)
							{
								cC = -1;
								for (size_t j = 0; j < clients.size() && cC == -1; j++)
								{
									if (clients.at(j).sockID == i)
									{
										cC = j;
									}
								}
								if (cC > -1 && !clients.at(cC).isInGame && !clients.at(cC).inQueue && !clients.at(cC).isReady && !clients.at(cC).spectating)
								{
									if (games.size() > 0)
									{
										send(clients.at(cC).sockID, "Choose a game to spectate, press '9' to leave.\n", strlen("Choose a game to spectate, press '9' to leave.\n"), 0);
										for (size_t j = 0; j < games.size(); j++)
										{
											clients.at(cC).spectating = true;
											memset(sendBuf, 0, sizeof(sendBuf));
											sprintf(sendBuf, "%ld\n", j + 1);
											send(clients.at(cC).sockID, sendBuf, strlen(sendBuf), 0);
											memset(recvBuf, 0, sizeof(recvBuf));
											sprintf(recvBuf, "-2");
										}
									}
									else
									{
										send(clients.at(cC).sockID, "No games right now, check again later.\n", strlen("No games right now, check again later.\n"), 0);
										send(clients.at(cC).sockID, MENU, strlen(MENU), 0);
										clients.at(cC).spectating = false;
										clients.at(cC).gameID = -1;
									}
								}
							}
							else if (strstr(recvBuf, "3") != nullptr)
							{
								cC = -1;
								for (size_t j = 0; j < clients.size() && cC == -1; j++)
								{
									if (clients.at(j).sockID == i)
									{
										cC = j;
									}
								}
								if (timeScore.size() > 0 && !clients.at(cC).isInGame && !clients.at(cC).inQueue && !clients.at(cC).isReady && !clients.at(cC).spectating)
								{
									for (size_t j = 0; j < timeScore.size(); j++)
									{
										for (size_t g = 0; g < timeScore.size(); g++)
										{
											if (timeScore.at(g) > timeScore.at(j))
											{
												size_t copy = timeScore.at(g);
												timeScore.at(g) = timeScore.at(j);
												timeScore.at(j) = copy;
											}
										}
									}
									for (size_t j = 0; j < timeScore.size(); j++)
									{
										memset(sendBuf, 0, sizeof(sendBuf));
										sprintf(sendBuf, "Avrage time of the winner was: %ld\n", timeScore.at(j));
										send(clients.at(cC).sockID, sendBuf, strlen(sendBuf), 0);
									}
									send(clients.at(cC).sockID, MENU, strlen(MENU), 0);
								}
							}
							else if (strstr(recvBuf, "9") != nullptr)
							{
								cC = -1;
								for (size_t j = 0; j < clients.size() && cC == -1; j++)
								{
									if (clients.at(j).sockID == i)
									{
										cC = j;
									}
								}
								if (cC > -1 && clients.at(cC).inQueue)
								{
									if (clients.at(cC).inQueue && !clients.at(cC).isReady && !clients.at(cC).isInGame)
									{
										cC = -1;
										for (size_t j = 0; j < queues.size() && cC == -1; j++)
										{
											if (queues.at(j)->sockID == i)
											{
												cC = j;
											}
										}
										queues.at(cC)->inQueue = false;
										send(queues.at(cC)->sockID, MENU, strlen(MENU), 0);
										queues.erase((queues.begin() + cC));
									}
								}
								else if (cC > -1 && clients.at(cC).spectating)
								{
									clients.at(cC).spectating = false;
									clients.at(cC).gameID = -1;
									send(clients.at(cC).sockID, MENU, strlen(MENU), 0);
								}
							}
							else if (strstr(recvBuf, "r") != nullptr)
							{
								cC = -1;
								for (size_t j = 0; j < clients.size() && cC == -1; j++)
								{
									if (clients.at(j).sockID == i)
									{
										cC = j;
									}
								}
								if (queues.size() == 2)
								{
									for (size_t j = 0; j < queues.size(); j++)
									{
										if (queues.at(j)->sockID == i)
										{
											queues.at(j)->inQueue = false;
											queues.at(j)->isReady = true;
										}
									}
								}
								if (queues.at(0)->isReady && !queues.at(1)->isReady)
								{
									send(queues.at(0)->sockID, "Waiting for other player to be ready.\n", strlen("Waiting for other player to be ready.\n"), 0);
									break;
								}
								else if (!queues.at(0)->isReady && queues.at(1)->isReady)
								{
									send(queues.at(1)->sockID, "Waiting for other player to be ready.\n", strlen("Waiting for other player to be ready.\n"), 0);
									break;
								}
								if (queues.at(0)->isReady && queues.at(1)->isReady)
								{
									struct game newGame;
									newGame.p1score = 0;
									newGame.p2score = 0;
									newGame.p1Option = 0;
									newGame.p2Option = 0;
									newGame.winner = -1;
									newGame.player1 = queues.at(0);
									newGame.player2 = queues.at(1);
									newGame.player1->isReady = true;
									newGame.player2->isReady = true;
									gettimeofday(&newGame.tid, NULL);
									games.push_back(&newGame);

									for (size_t j = 0; j < clients.size(); j++)
									{
										if (clients.at(j).sockID == queues.at(0)->sockID)
										{
											clients.at(j).gameID = games.size() - 1;
											gettimeofday(&clients.at(j).tid, NULL);
										}
										else if (clients.at(j).sockID == queues.at(1)->sockID)
										{
											clients.at(j).gameID = games.size() - 1;
											gettimeofday(&clients.at(j).tid, NULL);
										}
									}
									for (int b = 0; b < 2; b++)
									{
										queues.erase(queues.begin());
									}
									if (games.size() > 0)
									{
										for (size_t j = 0; j < games.size(); j++)
										{
											if (games.at(j)->player1->isReady && games.at(j)->player2->isReady)
											{
												games.at(j)->player1->isInGame = true;
												games.at(j)->player2->isInGame = true;
												for (size_t g = 0; g < clients.size(); g++)
												{
													if (clients.at(g).sockID == games.at(j)->player1->sockID)
													{
														clients.at(g).isInGame = true;
													}
													else if (clients.at(g).sockID == games.at(j)->player2->sockID)
													{
														clients.at(g).isInGame = true;
													}
												}
												games.at(j)->player1->isReady = false;
												games.at(j)->player2->isReady = false;
											}
										}
									}
								}
							}
							else if (strstr(recvBuf, "1") != nullptr || strstr(recvBuf, "2") != nullptr || strstr(recvBuf, "3") != nullptr) {}
							else
							{
								send(i, "ERROR Wrong format on the message sent.\n", strlen("ERROR Wrong format on the message sent.\n"), 0);
							}
						}
						if (games.size() > 0)
						{
							cC = -1;
							for (size_t j = 0; j < clients.size() && cC == -1; j++)
							{
								if (clients.at(j).sockID == i)
								{
									cC = j;
								}
							}
							char checking[256];
							size_t checker = -1;
							for (size_t j = 0; j < games.size(); j++)
							{
								for (size_t g = 0; g < clients.size(); g++)
								{
									if (clients.at(g).sockID == i)
									{
										cC = g;
									}
									for (size_t h = 0; h < games.size(); h++)
									{
										if (clients.at(cC).spectating && clients.at(cC).gameID == checker && clients.at(cC).sockID == i && !clients.at(cC).isInGame)
										{
											if (strstr(recvBuf, "9") != nullptr)
											{
												clients.at(cC).spectating = false;
												send(clients.at(cC).sockID, MENU, strlen(MENU), 0);
												memset(recvBuf, 0, sizeof(recvBuf));
												sprintf(recvBuf, "-2");
											}
											snprintf(checking, sizeof(checking), "%zu", j + 1);
											if (strcmp(recvBuf, checking) == 0)
											{
												clients.at(cC).gameID = j;
												memset(sendBuf, 0, sizeof(sendBuf));
												sprintf(sendBuf, "Spectating game: %ld. Press 9 to leave!\n", clients.at(cC).gameID);
												send(clients.at(cC).sockID, sendBuf, strlen(sendBuf), 0);
												memset(recvBuf, 0, sizeof(recvBuf));
												sprintf(recvBuf, "-2");
											}
										}
										else if (clients.at(cC).spectating && clients.at(cC).gameID == h && clients.at(cC).sockID == i)
										{
											if (strstr(recvBuf, "-9") != nullptr || (reciver > 0 && strcmp(recvBuf, "-2") != 0))
											{
												clients.at(cC).gameID = -1;
												send(clients.at(cC).sockID, "Choose a game to spectate, press 9 to leave.\n", strlen("Choose a game to spectate, press 9 to leave.\n"), 0);
												for (size_t k = 0; k < games.size(); k++)
												{
													memset(sendBuf, 0, sizeof(sendBuf));
													sprintf(sendBuf, "%ld\n", k + 1);
													send(clients.at(cC).sockID, sendBuf, strlen(sendBuf), 0);
													memset(recvBuf, 0, sizeof(recvBuf));
													sprintf(recvBuf, "-2");
												}
											}
										}
									}
									if (clients.at(g).spectating)
									{
										break;
									}
									else if ((gameTime.tv_sec - clients.at(g).tid.tv_sec) < 3 && ((gameTime.tv_sec - games.at(j)->tid.tv_sec) > 3) && clients.at(g).sockID == i && clients.at(g).isInGame && clients.at(g).gameID == j)
									{
										if (strstr(recvBuf, "1") != nullptr)
										{
											clients.at(g).answer = 1;
											gettimeofday(&clients.at(g).tid, NULL);
										}
										else if (strstr(recvBuf, "2") != nullptr)
										{
											clients.at(g).answer = 2;
											gettimeofday(&clients.at(g).tid, NULL);
										}
										else if (strstr(recvBuf, "3") != nullptr)
										{
											clients.at(g).answer = 3;
											gettimeofday(&clients.at(g).tid, NULL);
										}
									}
								}
							}
						}
						if (reciver <= 0)
						{
							close(i);
							for (size_t j = 0; j < clients.size(); j++)
							{
								if (i == clients.at(j).sockID)
								{
									clients.erase(clients.begin() + j);
									FD_CLR(i, &currentSockets);
									break;
								}
							}
						}
					}
				}
				FD_CLR(i, &readySockets);
			}
		}
	}
	close(sockfd);
	return 0;
}