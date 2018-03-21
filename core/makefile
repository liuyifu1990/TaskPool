.PHONY: Clean
SRC = _main.c queue.c utils.c log.c
OBJS = _main.o queue.o utils.o log.o
CC = gcc
CoreTest: $(OBJS)
	$(CC) -o CoreTest $(OBJS) -lpthread

_main.o: _main.c
	$(CC) -c -g _main.c

queue.o: queue.c 
	$(CC) -c -g  queue.c

utils.o: utils.c
	$(CC) -c -g  utils.c

log.o: log.c
	$(CC) -c -g log.c
Clean:
	rm -f *.o CoreTest
