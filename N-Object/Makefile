# 自动编译当前目录下所有C文件为对应的二进制文件
# Author: ChongKai
# Usage: make [options]
# options:
# 	D=DEFINE	#define DEFINE in C

SRC = $(wildcard *.c)
BIN = $(SRC:%.c=%)

CC = gcc
CFLAGS = -Wall -Werror -O0 -g
LDLIBS = -fopenmp -lpthread -lm

all: $(BIN)

ifndef D
$(BIN): %:%.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)
else
$(BIN): %:%.c
	$(CC) -D$(D) $(CFLAGS) $^ -o $@ $(LDLIBS)
endif

.PHONY: clean
clean:
	-rm -rf $(BIN) result1.txt result2.txt result3.txt result4.txt
