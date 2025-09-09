#ifndef JOBS_H
#define JOBS_H

#include <sys/types.h>

#define MAX_JOBS 100

typedef struct {
    int job_id;
    pid_t pid;
    char command[1024];
    int active;
} Job;

void initJobs(void);
int addJob(pid_t pid, char *command);
void removeJob(pid_t pid);
void checkBackgroundJobs(void);
void printJobStatus(pid_t pid, int status);

#endif
