#include <stdio.h>
#include <unistd.h>
#include "prompt.h"

char shell_home[1024];

int main()
{
    getcwd(shell_home, sizeof(shell_home));
    while (1)
    {
        display_prompt();
        getchar();
    }
    return 0;
}