#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "parser.h"

enum tokenType { 
    NAME, 
    OP, 
    END, 
    INVALID 
};

char *skipSpaces(char *s)
{
    while (*s && isspace((unsigned char)*s))
    {
        s++;
    }
    return s;
}

int isOperator(char c)
{
    return (c == '|' || c == '&' || c == ';' || c == '<' || c == '>');
}

enum tokenType nextToken(char **p, char *buffer, size_t bufferLen)
{
    char *s = skipSpaces(*p);

    if (*s == '\0') 
    {
        *p = s;
        return END; 
    }

    if (isOperator(*s))
    {
        size_t len = 1;
        buffer[0] = *s;
        buffer[1] = '\0';

        if (*s == '>' && *(s+1) == '>')
        {
            buffer[1] = '>';
            buffer[2] = '\0';
            len = 2;
        }
        *p = s + len;
        return OP;
    }
    else
    {
        size_t i = 0;
        while (*s && !isOperator(*s) && !isspace((unsigned char)*s))
        {
            if (i+1 < bufferLen)
            {
                buffer[i] = *s;
                i++;
            }
            s++;
        }
        buffer[i] = '\0';
        *p = s;
        if (i > 0)
        {
            return NAME;
        }
        else
        {
            return INVALID;
        }
    }
}

int parseCommand(char *input)
{
    char *p = input;
    char token[256];
    enum tokenType last = OP;

    while (1)
    {
        enum tokenType t = nextToken(&p, token, sizeof(token));
        if (t == END)
        {
            break;
        }
        if (t == INVALID)
        {
            printf("Invalid Syntax!\n");
            return -1;
        }
        
        if (last == OP && t == OP)
        {
            printf("Invalid Syntax!\n");
            return -1;
        }
        last = t;
    }

    if (last == OP)
    {
        printf("Invalid Syntax!\n");
        return -1;
    }
    
    return 0;
}