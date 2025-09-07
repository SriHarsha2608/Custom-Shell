#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <limits.h>
#include <errno.h>
#include "reveal.h"

extern char shellHome[];
extern char prevDir[];
extern int prevValid;

int compareStrings(const void *a, const void *b)
{
    return strcmp(*(const char **)a, *(const char **)b);
}

void getParentDir(char *path, char *parent)
{
    strcpy(parent, path);
    char *lastSlash = strrchr(parent, '/');
    if (lastSlash == NULL)
    {
        strcpy(parent, "/");
        return;
    }

    if (lastSlash == parent)
    {
        strcpy(parent, "/");
        return;
    }

    *lastSlash = '\0';
}

int dirExists(char *path)
{
    struct stat st;
    if (stat(path, &st) == 0)
    {
        return S_ISDIR(st.st_mode);
    }
    return 0;
}

void doReveal(int argc, char **argv)
{
    int showAll = 0;
    int lineByLine = 0;
    char targetDir[PATH_MAX];
    char currentDir[PATH_MAX];

    getcwd(currentDir, sizeof(currentDir));

    strcpy(targetDir, currentDir);

    int argIndex = 1;
    int pathArgCount = 0;

    while (argIndex < argc)
    {
        if (argv[argIndex][0] == '-' && argv[argIndex][1] != '\0')
        {
            for (int i = 1; argv[argIndex][i] != '\0'; i++)
            {
                if (argv[argIndex][i] == 'a')
                {
                    showAll = 1;
                }
                else if (argv[argIndex][i] == 'l')
                {
                    lineByLine = 1;
                }
                else
                {
                    printf("reveal: Invalid Syntax!\n");
                    return;
                }
            }
        }
        else
        {
            pathArgCount++;
            if (pathArgCount > 1)
            {
                printf("reveal: Invalid Syntax!\n");
                return;
            }

            if (strcmp(argv[argIndex], "~") == 0)
            {
                strcpy(targetDir, shellHome);
            }
            else if (strcmp(argv[argIndex], ".") == 0)
            {
                strcpy(targetDir, currentDir);
            }
            else if (strcmp(argv[argIndex], "..") == 0)
            {
                if (strcmp(currentDir, shellHome) == 0)
                {
                    strcpy(targetDir, currentDir);
                }
                else
                {
                    getParentDir(currentDir, targetDir);
                }
            }
            else if (strcmp(argv[argIndex], "-") == 0)
            {
                if (!prevValid)
                {
                    printf("No such directory!\n");
                    return;
                }
                strcpy(targetDir, prevDir);
            }
            else
            {
                if (argv[argIndex][0] == '/')
                {
                    strcpy(targetDir, argv[argIndex]);
                }
                else
                {
                    int len = snprintf(targetDir, sizeof(targetDir), "%s/%s", currentDir, argv[argIndex]);
                    if (len >= (int)sizeof(targetDir))
                    {
                        printf("reveal: Path too long!\n");
                        return;
                    }
                }
            }
        }
        argIndex++;
    }

    if (!dirExists(targetDir))
    {
        printf("No such directory!\n");
        return;
    }

    DIR *dir = opendir(targetDir);
    if (dir == NULL)
    {
        printf("No such directory!\n");
        return;
    }

    struct dirent *entry;
    char **fileNames = NULL;
    int fileCount = 0;
    int capacity = 16;

    fileNames = malloc(capacity * sizeof(char *));
    if (fileNames == NULL)
    {
        closedir(dir);
        perror("malloc");
        return;
    }

    errno = 0;
    while ((entry = readdir(dir)) != NULL)
    {
        if (!showAll && entry->d_name[0] == '.')
        {
            continue;
        }

        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        if (fileCount >= capacity)
        {
            capacity *= 2;
            char **temp = realloc(fileNames, capacity * sizeof(char *));
            if (temp == NULL)
            {
                for (int i = 0; i < fileCount; i++)
                {
                    free(fileNames[i]);
                }
                free(fileNames);
                closedir(dir);
                perror("realloc");
                return;
            }
            fileNames = temp;
        }

        size_t nameLen = strlen(entry->d_name);
        fileNames[fileCount] = malloc(nameLen + 1);
        if (fileNames[fileCount] == NULL)
        {
            for (int i = 0; i < fileCount; i++)
            {
                free(fileNames[i]);
            }
            free(fileNames);
            closedir(dir);
            perror("malloc");
            return;
        }
        strcpy(fileNames[fileCount], entry->d_name);
        fileCount++;
    }

    if (errno != 0)
    {
        for (int i = 0; i < fileCount; i++)
        {
            free(fileNames[i]);
        }
        free(fileNames);
        closedir(dir);
        perror("readdir");
        return;
    }

    closedir(dir);

    if (fileCount > 0)
    {
        qsort(fileNames, (size_t)fileCount, sizeof(char *), compareStrings);
    }

    if (lineByLine)
    {
        for (int i = 0; i < fileCount; i++)
        {
            printf("%s\n", fileNames[i]);
        }
    }
    else
    {
        for (int i = 0; i < fileCount; i++)
        {
            if (i > 0)
            {
                printf(" ");
            }
            printf("%s", fileNames[i]);
        }
        if (fileCount > 0)
        {
            printf("\n");
        }
    }

    for (int i = 0; i < fileCount; i++)
    {
        free(fileNames[i]);
    }
    free(fileNames);
}
