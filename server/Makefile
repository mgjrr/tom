CC = g++
CFLAGS = -std=c++11 -lpthread
 
all:  T_server_main.cpp  T_server.o
	$(CC) $(CFLAGS) T_server_main.cpp  T_server.o -o server_T
	
T_server.o: T_server.cpp T_server.h facility.h
	$(CC) $(CFLAGS) -c T_server.cpp

clean:
	rm -f *.o server_T