CC = g++
CFLAGS = -std=c++11 -lpthread -lhiredis -I /root
 
all:  T_server_main.cpp  T_server.o log.o thread_pool.o redis.o
	$(CC) $(CFLAGS) T_server_main.cpp  T_server.o log.o thread_pool.o redis.o   -o server_T 
	
T_server.o: T_server.cpp T_server.h facility.h
	$(CC) $(CFLAGS) -c T_server.cpp

log.o: log.cpp log.h
	$(CC) $(CFLAGS) -c log.cpp

thread_pool.o : thread_pool.cpp thread_pool.h
	$(CC) $(CFLAGS) -c thread_pool.cpp

redis.o : redis.cpp redis.h
	$(CC) $(CFLAGS) -c redis.cpp

clean:
	rm -f *.o server_T