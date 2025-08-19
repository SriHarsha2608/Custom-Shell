#include <stdio.h>
#include <unistd.h>
#include "prompt.h"
#include "input.h"
#include "parser.h"

char shell_home[1024];

int main()
{
    getcwd(shell_home, sizeof(shell_home));
    char input[1024];
    while (1)
    {
        display_prompt();
        int len = userInput(input, sizeof(input));
        // if (len <= 0)
        // {
        //     continue;
        // }
        parseCommand(input);
        // if (parseCommand(input) == 0)
        // {
            
        // }
        
    }
    return 0;
}