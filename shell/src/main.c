#include <stdio.h>
#include <unistd.h>
#include "prompt.h"
#include "input.h"

char shell_home[1024];

int main()
{
    getcwd(shell_home, sizeof(shell_home));
    char input[1024];
    while (1)
    {
        display_prompt();
        userInput(input, sizeof(input));
    }
    return 0;
}