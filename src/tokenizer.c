#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "tokenizer.h"

void tokenize(char *input, token *tokens, int *count)
{
    int i = 0, t = 0;
    while (input[i] != '\0')
    {
        if (isspace((unsigned char)input[i]))
        {
            i++;
            continue;
        }
        if (input[i] == '|')
        {
            tokens[t].type = T_PIPE;
            strcpy(tokens[t].value, "|");
            t++;
            i++;
        }
        else if (input[i] == '&')
        {
            tokens[t].type = T_AND;
            strcpy(tokens[t].value, "&");
            t++;
            i++;
        }
        else if (input[i] == ';')
        {
            tokens[t].type = T_SEMI;
            strcpy(tokens[t].value, ";");
            t++;
            i++;
        }
        else if (input[i] == '<')
        {
            tokens[t].type = T_INPUT;
            strcpy(tokens[t].value, "<");
            t++;
            i++;
        }
        else if (input[i] == '>')
        {
            if (input[i + 1] == '>')
            {
                tokens[t].type = T_APPEND;
                strcpy(tokens[t].value, ">>");
                t++;
                i += 2;
            }
            else
            {
                tokens[t].type = T_OUTPUT;
                strcpy(tokens[t].value, ">");
                t++;
                i++;
            }
        }
        else
        {
            int j = 0;
            while (input[i] != '\0' && !isspace((unsigned char)input[i]) && input[i] != '|' && input[i] != '&' && input[i] != ';' && input[i] != '<' && input[i] != '>')
            {
                tokens[t].value[j++] = input[i++];
            }
            tokens[t].value[j] = '\0';
            tokens[t].type = T_NAME;
            t++;
        }
    }
    tokens[t].type = T_END;
    tokens[t].value[0] = '\0';
    *count = t;
}