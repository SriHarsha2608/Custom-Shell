#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>
#include "jobs.h"

// Job management
static Job jobs[MAX_JOBS];
static int next_job_id = 1;

// Helper function to extract command name
static char* extractCommandName(const char *full_command) {
    static char cmd_name[256];
    int i = 0, j = 0;
    
    // Skip leading whitespace
    while (full_command[i] && (full_command[i] == ' ' || full_command[i] == '\t')) {
        i++;
    }
    
    // Extract first word
    while (full_command[i] && full_command[i] != ' ' && full_command[i] != '\t' && j < 255) {
        cmd_name[j++] = full_command[i++];
    }
    cmd_name[j] = '\0';
    
    return cmd_name;
}

void initJobs(void) {
    for (int i = 0; i < MAX_JOBS; i++) {
        jobs[i].active = 0;
        jobs[i].job_id = 0;
        jobs[i].pid = 0;
        jobs[i].pgid = 0;
        jobs[i].command[0] = '\0';
        jobs[i].state = JOB_DONE;
    }
}

int addJob(pid_t pid, char *command) {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (!jobs[i].active) {
            jobs[i].active = 1;
            jobs[i].job_id = next_job_id++;
            jobs[i].pid = pid;
            jobs[i].pgid = pid;  // Process group ID same as PID for job leader
            jobs[i].state = JOB_RUNNING;
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

// Update job state
void updateJobState(pid_t pid, JobState state) {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs[i].active && jobs[i].pid == pid) {
            jobs[i].state = state;
            break;
        }
    }
}

// Find job by job ID
Job* findJob(int job_id) {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs[i].active && jobs[i].job_id == job_id) {
            return &jobs[i];
        }
    }
    return NULL;
}

// Find job by PID
Job* findJobByPid(pid_t pid) {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs[i].active && jobs[i].pid == pid) {
            return &jobs[i];
        }
    }
    return NULL;
}

// Get last job ID
int getLastJobId(void) {
    int last_id = -1;
    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs[i].active && jobs[i].job_id > last_id) {
            last_id = jobs[i].job_id;
        }
    }
    return last_id;
}

void printJobStatus(pid_t pid, int status) {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs[i].active && jobs[i].pid == pid) {
            char *cmd_name = extractCommandName(jobs[i].command);
            
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






