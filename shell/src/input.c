// #include <stdio.h>
// #include <string.h>
// #include <unistd.h>
// #include "input.h"

// ssize_t userInput(char *buffer, int size)
// {
//     ssize_t len = read(STDIN_FILENO, buffer, size - 1);
//     if (len > 0) 
//     {
//         buffer[len - 1] = '\0';
//     } 
//     else if (len == 0) 
//     {
//         buffer[0] = '\0'; 
//     }
//     return len;
// }

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h> 
#include <errno.h>
#include "input.h"
#include "jobs.h"

extern volatile sig_atomic_t child_exited;

ssize_t userInput(char *buffer, int size) {
    while (1) {
        // Check for completed background jobs before reading input
        if (child_exited) {
            checkBackgroundJobs();
            child_exited = 0;
        }
        
        ssize_t len = read(STDIN_FILENO, buffer, size - 1);
        
        if (len > 0) {
            buffer[len - 1] = '\0';
            return len;
        } else if (len == 0) {
            buffer[0] = '\0';
            return len;
        } else if (errno == EINTR) {
            // System call was interrupted by signal (SIGCHLD)
            // Check for completed jobs and continue reading
            if (child_exited) {
                checkBackgroundJobs();
                child_exited = 0;
            }
            continue; // Retry the read
        } else {
            // Other error
            perror("read");
            buffer[0] = '\0';
            return -1;
        }
    }
}
