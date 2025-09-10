# CC = gcc
# CFLAGS = -Wall -Iinclude
# SRC = src/main.c src/prompt.c src/input.c src/parser.c src/tokenizer.c src/hop.c src/reveal.c
# OBJ = $(SRC:.c=.o)
# TARGET = shell

# $(TARGET): $(OBJ)
# 	$(CC) $(CFLAGS) -o $@ $^

# clean:
# 	rm -f $(OBJ) $(TARGET)

CC = gcc
CFLAGS = -std=c99 -D_POSIX_C_SOURCE=200809L -D_XOPEN_SOURCE=700 -Wall -Wextra -Werror -Wno-unused-parameter -fno-asm

SRCDIR = src
INCDIR = include
SOURCES = main.c prompt.c input.c tokenizer.c parser.c hop.c reveal.c executor.c log.c jobs.c ping.c
OBJECTS = $(SOURCES:.c=.o)

all: shell.out

shell.out: $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -I$(INCDIR) -c $< -o $@

clean:
	rm -f *.o shell.out

.PHONY: all clean
