#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE 700
#include <stdio.h>
#include "tokenizer.h"
#include "parser.h"

token *tokens;
int pos;

token *current()
{
    return &tokens[pos];
}

void advance()
{
    if (tokens[pos].type != T_END)
    {
        pos++;
    }
}

int parse_atomic();
int parse_cmd_group();
int parse_shell_cmd();

int parse(token *toks)
{
    tokens = toks;
    pos = 0;
    int ok = parse_shell_cmd();
    return ok && current()->type == T_END;
}

int parse_shell_cmd()
{
    if (!parse_cmd_group())
    {
        return 0;
    }
    while (current()->type == T_AND || current()->type == T_SEMI)
    {
        tokenType t = current()->type;
        advance();

        if (t == T_SEMI || (t == T_AND && current()->type != T_END))
        {
            if (!parse_cmd_group())
            {
                return 0;
            }
        }
    }
    if (current()->type == T_AND)
    {
        advance();
    }

    return 1;
}

int parse_cmd_group()
{
    if (!parse_atomic())
    {
        return 0;
    }
    while (current()->type == T_PIPE)
    {
        advance();
        if (!parse_atomic())
        {
            return 0;
        }
    }
    return 1;
}

int parse_atomic()
{
    if (current()->type != T_NAME)
    {
        return 0;
    }
    advance();

    while (current()->type == T_NAME || current()->type == T_INPUT || current()->type == T_OUTPUT || current()->type == T_APPEND)
    {
        // advance();
        if (current()->type == T_NAME)
        {
            advance();
        }
        else if (current()->type == T_INPUT || current()->type == T_OUTPUT || current()->type == T_APPEND)
        {
            advance();
            if (current()->type != T_NAME)
            {
                return 0;
            }
            advance();
        }
    }

    return 1;
}