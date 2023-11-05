CC=gcc
CFLAGS=-g -pedantic -std=gnu17 -Wall -Werror -Wextra

.PHONY: all
all: nyush

nyush: nyush.o functions.o
	$(CC) $(CFLAGS) -o nyush nyush.o functions.o

nyush.o: nyush.c functions.h
	$(CC) $(CFLAGS) -c nyush.c

functions.o: functions.c functions.h
	$(CC) $(CFLAGS) -c functions.c

.PHONY: clean
clean:
	rm -f *.o nyush