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
    char displayPath[1024];

    size_t homeLen = strlen(shell_home);

    if (strncmp(cwd, shell_home, homeLen) == 0) 
    {
        if (cwd[homeLen] == '\0') 
        {
            snprintf(displayPath, sizeof(displayPath), "~");
        } 
        else if (cwd[homeLen] == '/') 
        {
            snprintf(displayPath, sizeof(displayPath), "~%s", cwd + homeLen);
        } 
        else 
        {
            snprintf(displayPath, sizeof(displayPath), "%s", cwd);
        }
    } else {
        snprintf(displayPath, sizeof(displayPath), "%s", cwd);
    }

    printf("<%s@%s:%s> ", username, hostname, displayPath);
    fflush(stdout);
}