#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include "ping.h"

// // ############## LLM Generated Code Starts ################
void doPing(int argc, char **argv)
{
    if (argc != 3)
    {
        printf("Invalid syntax!\n");
        return;
    }

    // Parse PID
    char *endptr;
    long pid_long = strtol(argv[1], &endptr, 10);
    if (*endptr != '\0' || pid_long <= 0)
    {
        printf("Invalid syntax!\n");
        return;
    }
    pid_t pid = (pid_t)pid_long;

    // Parse signal number
    long signal_long = strtol(argv[2], &endptr, 10);
    if (*endptr != '\0')
    {
        printf("Invalid syntax!\n");
        return;
    }

    // Take signal number modulo 32
    int signal_num = (int)(signal_long % 32);

    // Send signal to process
    if (kill(pid, signal_num) == -1)
    {
        printf("No such process found\n");
    }
    else
    {
        printf("Sent signal %ld to process with pid %ld\n", signal_long, pid_long);
    }
    // ############## LLM Generated Code Ends ################
}
