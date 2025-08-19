#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "input.h"

ssize_t userInput(char *buffer, int size)
{
    ssize_t len = read(STDIN_FILENO, buffer, size - 1);
    if (len > 0) 
    {
        buffer[len - 1] = '\0';
    } 
    else if (len == 0) 
    {
        buffer[0] = '\0'; 
    }
    return len;
}