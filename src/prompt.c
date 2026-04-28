#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <string.h>
#include <unistd.h>
#include "prompt.h"
#include "hop.h" // Add this include

void display_prompt()
{
    char hostname[256];
    char cwd[1024];

    struct passwd *pw = getpwuid(getuid());
    char *username = pw ? pw->pw_name : "user";

    if (gethostname(hostname, sizeof(hostname)) == -1)
    {
        perror("gethostname");
        strncpy(hostname, "unknown", sizeof(hostname) - 1);
        hostname[sizeof(hostname) - 1] = '\0';
    }
    if (getcwd(cwd, sizeof(cwd)) == NULL)
    {
        perror("getcwd");
        strncpy(cwd, "unknown", sizeof(cwd) - 1);
        cwd[sizeof(cwd) - 1] = '\0';
    }

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
    }
    else
    {
        snprintf(displayPath, sizeof(displayPath), "%s", cwd);
    }

    printf("<%s@%s:%s> ", username, hostname, displayPath);
    fflush(stdout);
}