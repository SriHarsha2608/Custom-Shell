#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "input.h"

void userInput(char *buffer, int size)
{
    ssize_t bytes_read = read(STDIN_FILENO, buffer, size - 1);
    if (bytes_read <= 0) {
        buffer[0] = '\0';
        return;
    }

    buffer[bytes_read] = '\0';

    char *newline = strchr(buffer, '\n');
    if (newline) 
    {
        *newline = '\0';
    }
}