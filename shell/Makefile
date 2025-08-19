CC = gcc
CFLAGS = -Wall -Iinclude
SRC = src/main.c src/prompt.c src/input.c src/parser.c
OBJ = $(SRC:.c=.o)
TARGET = shell

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(OBJ) $(TARGET)