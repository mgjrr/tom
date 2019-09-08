CC = g++
CFLAGS = -std=c++11

all: T_client_main.cpp T_client.o  log.o
	$(CC) $(CFLAGS) T_client_main.cpp T_client.o log.o -o client_T

T_client.o: T_client.cpp T_client.h facility.h
	$(CC) $(CFLAGS) -c T_client.cpp


log.o: log.cpp log.h
	$(CC) $(CFLAGS) -c log.cpp

clean:
	rm -f *.o client_T