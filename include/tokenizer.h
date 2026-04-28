#ifndef TOKENIZER_H
#define TOKENIZER_H

typedef enum
{
    T_NAME,
    T_PIPE,
    T_AND,
    T_SEMI,
    T_INPUT,
    T_OUTPUT,
    T_APPEND,
    T_END
} tokenType;

typedef struct
{
    tokenType type;
    char value[256];
} token;

void tokenize(char *input, token *tokens, int *count);

#endif