CC = g++
CFLAGS = -std=c++11
 
all: T_client_main.cpp T_client.o
	$(CC) $(CFLAGS) T_client_main.cpp T_client.o -o client_T
 
T_client.o: T_client.cpp T_client.h facility.h
	$(CC) $(CFLAGS) -c T_client.cpp
 
clean:
	rm -f *.o client_T