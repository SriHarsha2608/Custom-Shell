#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include "hop.h"

char shellHome[PATH_MAX];
char prevDir[PATH_MAX];
int prevValid = 0;

void initHop()
{
    getcwd(shellHome, sizeof(shellHome));
    prevValid = 0;
}

void doHop(int argc, char **argv)
{
    char cwd[PATH_MAX];
    if (argc == 1)
    {
        getcwd(cwd, sizeof(cwd));
        if (chdir(shellHome) == 0)
        {
            strcpy(prevDir, cwd);
            prevValid = 1;
        }
        return;
    }
    
    for (int i = 1; i < argc; i++)
    {
        char *arg = argv[i];

        if (strcmp(arg, "~") == 0)
        {
            getcwd(cwd, sizeof(cwd));
            if (chdir(shellHome) == 0)
            {
                strcpy(prevDir, cwd);
                prevValid = 1;
            }   
        }
        else if (strcmp(arg, ".") == 0)
        {
            continue;
        }
        
        else if (strcmp(arg, "..") == 0)
        {
            getcwd(cwd, sizeof(cwd));
            if (strcmp(cwd, shellHome) == 0)
            {
                continue;
            }
            
            if (chdir("..") == 0)
            {
                strcpy(prevDir, cwd);
                prevValid = 1;
            }
            
        }
        else if (strcmp(arg, "-") == 0)
        {
            if (prevValid)
            {
                getcwd(cwd, sizeof(cwd));
                char tempDir[PATH_MAX];
                strcpy(tempDir, prevDir);
                if (chdir(prevDir) == 0)
                {
                    strcpy(prevDir, cwd); 
                }
            }
            
        }
        else
        {
            getcwd(cwd, sizeof(cwd));
            if (chdir(arg) == 0)
            {
                strcpy(prevDir, cwd);
                prevValid = 1;
            }
            else
            {
                printf("No such directory!\n");
                fflush(stdout);
                // break;
            }
        }

        // if (prevValid)
        // {
        //     getcwd(cwd, sizeof(cwd));
        //     strcpy(prevDir, cwd);
        // }
        
        
    }
    
}
