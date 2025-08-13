#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <string.h>
#include <unistd.h>
#include "prompt.h"

extern char shell_home[];

void display_prompt()
{
    char hostname[256];
    char cwd[1024];

    struct passwd *pw = getpwuid(getuid());
    char *username = pw->pw_name;
    
    gethostname(hostname, sizeof(hostname));
    getcwd(cwd, sizeof(cwd));

    // char *home = getenv("HOME");

    if (strstr(cwd, shell_home) == cwd)
    {
        printf("%s@%s:~%s> ", username, hostname, cwd + strlen(shell_home));
    }
    else
    {
        printf("%s@%s:%s> ", username, hostname, cwd);
    }
    fflush(stdout);
}