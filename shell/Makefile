CC = gcc
CFLAGS = -Wall -Iinclude
SRC = src/main.c src/prompt.c src/input.c src/parser.c src/tokenizer.c src/hop.c src/reveal.c
OBJ = $(SRC:.c=.o)
TARGET = shell

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(OBJ) $(TARGET)