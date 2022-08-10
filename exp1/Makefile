.PHONY: all  clean
SRC = $(wildcard *.c)
BIN = $(SRC:%.c=%)
 
CC = gcc
CFLAGS = -Wall -g -lpthread
 
all:$(BIN)
 
clean:
	rm  $(BIN)