.PHONY: Clean
SRC = _main.c queue.c utils.c log.c timer.c
OBJS = _main.o queue.o utils.o log.o timer.o
CC = gcc
taskPool: $(OBJS)
	$(CC) -o CoreTest $(OBJS) -lpthread

_main.o: _main.c
	$(CC) -c -g _main.c

queue.o: queue.c 
	$(CC) -c -g  queue.c

utils.o: utils.c
	$(CC) -c -g  utils.c

log.o: log.c
	$(CC) -c -g log.c

timer.o: timer.c
	$(CC) -c -g timer.c

Clean:
	rm -f *.o CoreTest
