CC=gcc
CFLAGS=-Wall -Werror -W
SRC= $(wildcard *.c)
OBJS= $(SRC:.c=.o)

all: prog

makefile.dep: $(SRC)
	gcc -MM $^ > $@

prog: $(OBJS)
	gcc -o $@ $^

include makefile.dep
