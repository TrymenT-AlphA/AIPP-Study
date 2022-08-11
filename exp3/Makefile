.PHONY: all  clean  log
SRC = $(wildcard *.c)
BIN = $(SRC:%.c=%)

CC = gcc
CFLAGS = -Wall -g -lpthread

all:
	$(CC) $(CFLAGS) -o $(BIN) $(SRC)

log:
	$(CC) -DLOG $(CFLAGS) -o $(BIN) $(SRC)

clean:
	rm  $(BIN)
