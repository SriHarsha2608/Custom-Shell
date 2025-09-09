#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include "jobs.h"

static Job jobs[MAX_JOBS];
static int next_job_id = 1;

void initJobs(void) {
    for (int i = 0; i < MAX_JOBS; i++) {
        jobs[i].active = 0;
        jobs[i].job_id = 0;
        jobs[i].pid = 0;
        jobs[i].command[0] = '\0';
    }
}

int addJob(pid_t pid, char *command) {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (!jobs[i].active) {
            jobs[i].active = 1;
            jobs[i].job_id = next_job_id++;
            jobs[i].pid = pid;
            strncpy(jobs[i].command, command, sizeof(jobs[i].command) - 1);
            jobs[i].command[sizeof(jobs[i].command) - 1] = '\0';
            
            printf("[%d] %d\n", jobs[i].job_id, pid);
            fflush(stdout);
            return jobs[i].job_id;
        }
    }
    return -1; // No space for new job
}

void removeJob(pid_t pid) {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs[i].active && jobs[i].pid == pid) {
            jobs[i].active = 0;
            break;
        }
    }
}

char* getCommandName(char *command) {
    static char cmd_name[256];
    int i = 0;
    
    // Skip leading whitespace
    while (command[i] && (command[i] == ' ' || command[i] == '\t')) {
        i++;
    }
    
    // Extract first word (command name)
    int j = 0;
    while (command[i] && command[i] != ' ' && command[i] != '\t' && j < 255) {
        cmd_name[j++] = command[i++];
    }
    cmd_name[j] = '\0';
    
    return cmd_name;
}

void printJobStatus(pid_t pid, int status) {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs[i].active && jobs[i].pid == pid) {
            char *cmd_name = getCommandName(jobs[i].command);
            
            printf("\n");
            if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                printf("%s with pid %d exited normally\n", cmd_name, pid);
            } else {
                printf("%s with pid %d exited abnormally\n", cmd_name, pid);
            }
            fflush(stdout);
            break;
        }
    }
}

void checkBackgroundJobs(void) {
    int status;
    pid_t pid;
    
    // Check for completed background processes (non-blocking)
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        printJobStatus(pid, status);
        removeJob(pid);
    }
}
