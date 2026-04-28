#ifndef JOBS_H
#define JOBS_H

#include <sys/types.h>

#define MAX_JOBS 100

typedef enum {
    JOB_RUNNING,
    JOB_STOPPED,
    JOB_DONE
} JobState;

typedef struct {
    int job_id;
    pid_t pid;
    pid_t pgid;  // Process group ID for job control
    char command[1024];
    int active;
    JobState state;
} Job;


// Basic job management
void initJobs(void);
int addJob(pid_t pid, char *command);
int addBackgroundJob(pid_t pid, char *command);
void removeJob(pid_t pid);
void checkBackgroundJobs(void);
void printJobStatus(pid_t pid, int status);

// Advanced job control functions
void updateJobState(pid_t pid, JobState state);
Job* findJob(int job_id);
Job* findJobByPid(pid_t pid);
int getLastJobId(void);

// Helper function to extract command name
char* extractCommandName(const char *full_command);

void doActivities(void);

void doFg(int argc, char **argv);
void doBg(int argc, char **argv);

#endif
