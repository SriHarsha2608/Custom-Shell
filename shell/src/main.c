#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "prompt.h"
#include "input.h"
#include "parser.h"
#include "tokenizer.h"
#include "hop.h"

char shell_home[1024];

int main()
{
    getcwd(shell_home, sizeof(shell_home));
    initHop();

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
        
        if (tokens[0].type == T_NAME && strcmp(tokens[0].value, "hop") == 0)
        {
            int argc = 0;
            char *argv[32];
            for (int i = 0; i < count && tokens[i].type != T_END; i++)
            {
                argv[argc++] = tokens[i].value;
            }
            doHop(argc, argv);
        }
    }
    return 0;
}