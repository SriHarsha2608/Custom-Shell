#include <stdio.h>
#include <unistd.h>
#include "prompt.h"
#include "input.h"
#include "parser.h"
#include "tokenizer.h"

char shell_home[1024];

int main()
{
    getcwd(shell_home, sizeof(shell_home));
    char input[1024];
    token tokens[256];
    int count;

    while (1)
    {
        display_prompt();
        int len = userInput(input, sizeof(input));
        
        tokenize(input, tokens, &count);
        if (!parse(tokens))
        {
            printf("Invalid Syntax!\n");
        }
        
        
    }
    return 0;
}