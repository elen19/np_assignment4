all: client server


client.o: client.cpp
	$(CXX) -Wall -c client.cpp -I.
	
server.o: server.cpp
	$(CXX) -Wall -c server.cpp -I.
	

client: client.o
	$(CXX) -I./ -Wall -o sspgame client.o

server: server.o
	$(CXX) -I./ -Wall -o sspd server.o


clean:
	rm *.o sspgame sspd
