// ############## LLM Generated Code Begins ##############
#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE 700
// ############## LLM Generated Code Ends ################
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include "hop.h"

char prevDir[PATH_MAX];
int prevValid = 0;

void initHop()
{
    if (getcwd(shell_home, sizeof(shell_home)) == NULL)
    { // Changed from shellHome
        perror("getcwd");
        exit(EXIT_FAILURE);
    }
    prevValid = 0;
}

void doHop(int argc, char **argv)
{
    char cwd[PATH_MAX];
    if (argc == 1)
    {
        if (getcwd(cwd, sizeof(cwd)) == NULL)
        {
            perror("getcwd");
            return;
        }
        if (chdir(shell_home) == 0) // Changed from shellHome
        {
            strncpy(prevDir, cwd, sizeof(prevDir) - 1);
            prevDir[sizeof(prevDir) - 1] = '\0';
            prevValid = 1;
        }
        return;
    }

    for (int i = 1; i < argc; i++)
    {
        char *arg = argv[i];

        if (strcmp(arg, "~") == 0)
        {
            if (getcwd(cwd, sizeof(cwd)) == NULL)
            {
                perror("getcwd");
                continue;
            }
            if (chdir(shell_home) == 0) // Changed from shellHome
            {
                strncpy(prevDir, cwd, sizeof(prevDir) - 1);
                prevDir[sizeof(prevDir) - 1] = '\0';
                prevValid = 1;
            }
        }
        else if (strcmp(arg, ".") == 0)
        {
            continue;
        }

        else if (strcmp(arg, "..") == 0)
        {
            if (getcwd(cwd, sizeof(cwd)) == NULL)
            {
                perror("getcwd");
                continue;
            }
            // if (strcmp(cwd, shell_home) == 0) // Changed from shellHome
            // {
            //     continue;
            // }

            if (chdir("..") == 0)
            {
                strncpy(prevDir, cwd, sizeof(prevDir) - 1);
                prevDir[sizeof(prevDir) - 1] = '\0';
                prevValid = 1;
            }
        }
        else if (strcmp(arg, "-") == 0)
        {
            if (prevValid)
            {
                if (getcwd(cwd, sizeof(cwd)) == NULL)
                {
                    perror("getcwd");
                    continue;
                }
                char tempDir[PATH_MAX];
                strncpy(tempDir, prevDir, sizeof(tempDir) - 1);
                tempDir[sizeof(tempDir) - 1] = '\0';
                if (chdir(prevDir) == 0)
                {
                    strncpy(prevDir, cwd, sizeof(prevDir) - 1);
                    prevDir[sizeof(prevDir) - 1] = '\0';
                }
            }
        }
        else
        {
            if (getcwd(cwd, sizeof(cwd)) == NULL)
            {
                perror("getcwd");
                continue;
            }
            if (chdir(arg) == 0)
            {
                strncpy(prevDir, cwd, sizeof(prevDir) - 1);
                prevDir[sizeof(prevDir) - 1] = '\0';
                prevValid = 1;
            }
            else
            {
                printf("No such directory!\n");
                fflush(stdout);
            }
        }
    }
}