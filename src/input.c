#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include "input.h"
#include "jobs.h"
#include "signals.h"

extern volatile sig_atomic_t child_exited;

ssize_t userInput(char *buffer, int size)
{
    while (1)
    {
        ssize_t len = read(STDIN_FILENO, buffer, size - 1);

        if (len > 0)
        {
            // Null terminate
            buffer[len] = '\0';

            // Remove trailing newline and whitespace
            while (len > 0 && (buffer[len - 1] == '\n' || buffer[len - 1] == '\r' ||
                               buffer[len - 1] == ' ' || buffer[len - 1] == '\t'))
            {
                buffer[--len] = '\0';
            }

            return len;
        }
        else if (len == 0)
        {
            // EOF detected - set flag to indicate immediate exit needed
            should_exit_on_eof = 1;
            buffer[0] = '\0';
            return len;
        }
        else if (errno == EINTR)
        {
            // System call was interrupted by signal (SIGCHLD)
            // Just continue reading - job checking will happen in main loop
            continue;
        }
        else
        {
            // Other error
            perror("read");
            buffer[0] = '\0';
            return -1;
        }
    }
}
