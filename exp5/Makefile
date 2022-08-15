# 自动编译当前目录下所有C文件为对应的二进制文件
# Author: ChongKai
# Usage: make [options]
# options:
# 	D=DEFINE	#define DEFINE in C

SRC = $(wildcard *.c)
BIN = $(SRC:%.c=%)

CC = gcc
CFLAGS = -Wall -O0 -g -lpthread

all: $(BIN)

ifndef D
$(BIN): %:%.c
	$(CC) $(CFLAGS) $^ -o $@
else
$(BIN): %:%.c
	$(CC) -D$(D) $(CFLAGS) $^ -o $@
endif

.PHONY: clean
clean:
	-rm $(BIN)
