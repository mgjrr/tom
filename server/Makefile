CC = g++
CFLAGS = -std=c++11 -lpthread
 
all:  ServerMain.cpp  Server.o
	$(CC) $(CFLAGS) ServerMain.cpp  Server.o -o server
 
Server.o: Server.cpp Server.h Common.h
	$(CC) $(CFLAGS) -c Server.cpp
 
clean:
	rm -f *.o server