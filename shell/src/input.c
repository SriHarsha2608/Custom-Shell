#include <stdio.h>
#include <string.h>
#include "input.h"

void userInput(char *buffer, int size)
{
    if (!fgets(buffer, size, stdin))
    {
        buffer[0] = '\0';
        return ;
    }
    
    buffer[strcspn(buffer, "\n")] = '\0';
}