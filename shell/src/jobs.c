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
#include "signals.h"

// Job management
static Job jobs[MAX_JOBS];
static int next_job_id = 1;

// Helper function to extract command name
char *extractCommandName(const char *full_command)
{
    static char cmd_name[256];
    int i = 0, j = 0;

    // Skip leading whitespace
    while (full_command[i] && (full_command[i] == ' ' || full_command[i] == '\t'))
    {
        i++;
    }

    // Extract first word
    while (full_command[i] && full_command[i] != ' ' && full_command[i] != '\t' && j < 255)
    {
        cmd_name[j++] = full_command[i++];
    }
    cmd_name[j] = '\0';

    return cmd_name;
}

void initJobs(void)
{
    for (int i = 0; i < MAX_JOBS; i++)
    {
        jobs[i].active = 0;
        jobs[i].job_id = 0;
        jobs[i].pid = 0;
        jobs[i].pgid = 0;
        jobs[i].command[0] = '\0';
        jobs[i].state = JOB_DONE;
    }
}

int addJob(pid_t pid, char *command)
{
    for (int i = 0; i < MAX_JOBS; i++)
    {
        if (!jobs[i].active)
        {
            jobs[i].active = 1;
            jobs[i].job_id = next_job_id++;
            jobs[i].pid = pid;
            jobs[i].pgid = pid; // Process group ID same as PID for job leader
            jobs[i].state = JOB_RUNNING;
            strncpy(jobs[i].command, command, sizeof(jobs[i].command) - 1);
            jobs[i].command[sizeof(jobs[i].command) - 1] = '\0';

            return jobs[i].job_id;
        }
    }
    return -1; // No space for new job
}

// Add job for background processes (prints job info)
int addBackgroundJob(pid_t pid, char *command)
{
    int job_id = addJob(pid, command);
    if (job_id != -1)
    {
        printf("[%d] %d\n", job_id, pid);
        fflush(stdout);
    }
    return job_id;
}

void removeJob(pid_t pid)
{
    for (int i = 0; i < MAX_JOBS; i++)
    {
        if (jobs[i].active && jobs[i].pid == pid)
        {
            jobs[i].active = 0;
            break;
        }
    }
}

// Update job state
void updateJobState(pid_t pid, JobState state)
{
    for (int i = 0; i < MAX_JOBS; i++)
    {
        if (jobs[i].active && jobs[i].pid == pid)
        {
            jobs[i].state = state;
            break;
        }
    }
}

// Find job by job ID
Job *findJob(int job_id)
{
    for (int i = 0; i < MAX_JOBS; i++)
    {
        if (jobs[i].active && jobs[i].job_id == job_id)
        {
            return &jobs[i];
        }
    }
    return NULL;
}

// Find job by PID
Job *findJobByPid(pid_t pid)
{
    for (int i = 0; i < MAX_JOBS; i++)
    {
        if (jobs[i].active && jobs[i].pid == pid)
        {
            return &jobs[i];
        }
    }
    return NULL;
}

// Get last job ID
int getLastJobId(void)
{
    int last_id = -1;
    for (int i = 0; i < MAX_JOBS; i++)
    {
        if (jobs[i].active && jobs[i].job_id > last_id)
        {
            last_id = jobs[i].job_id;
        }
    }
    return last_id;
}

void printJobStatus(pid_t pid, int status)
{
    for (int i = 0; i < MAX_JOBS; i++)
    {
        if (jobs[i].active && jobs[i].pid == pid)
        {
            // Print the full original command (as typed)
            char *cmd_name = extractCommandName(jobs[i].command);
            if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
            {
                printf("%s with pid %d exited normally\n", cmd_name, pid);
            }
            else
            {
                printf("%s with pid %d exited abnormally\n", cmd_name, pid);
            }
            fflush(stdout);
            break;
        }
    }
}

void checkBackgroundJobs(void)
{
    int status;
    pid_t pid;

    // Check for completed background processes (non-blocking)
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        printJobStatus(pid, status);
        removeJob(pid);
    }
}

// ############## LLM Generated Code Starts ################
void doFg(int argc, char **argv)
{
    Job *job = NULL;

    if (argc == 1)
    {
        int last_id = getLastJobId();
        if (last_id == -1)
        {
            printf("No such job\n");
            return;
        }
        job = findJob(last_id);
    }
    else if (argc == 2)
    {
        char *endptr;
        long job_id_long = strtol(argv[1], &endptr, 10);
        if (*endptr != '\0' || job_id_long <= 0)
        {
            printf("No such job\n");
            return;
        }
        job = findJob((int)job_id_long);
    }
    else
    {
        printf("No such job\n");
        return;
    }

    if (!job)
    {
        printf("No such job\n");
        return;
    }

    // Print the entire command when bringing to foreground
    printf("%s\n", job->command);

    // If stopped, resume
    if (job->state == JOB_STOPPED)
    {
        kill(-job->pgid, SIGCONT);
    }

    // Set as foreground pgid so SIGINT/SIGTSTP are forwarded
    setForegroundPgid(job->pgid);

    // Mark as running
    job->state = JOB_RUNNING;

    // Wait for job to complete or stop again
    int status;
    pid_t res = waitpid(job->pid, &status, WUNTRACED);
    if (res > 0)
    {
        if (WIFSTOPPED(status))
        {
            job->state = JOB_STOPPED;
        }
        else
        {
            // Completed
            removeJob(job->pid);
        }
    }

    // Return terminal notionally to shell (we never transferred it)
    returnTerminalToShell();
}

// E.4: bg command - resume a stopped job in background
void doBg(int argc, char **argv)
{
    Job *job = NULL;

    if (argc == 1)
    {
        int last_id = getLastJobId();
        if (last_id == -1)
        {
            printf("No such job\n");
            return;
        }
        job = findJob(last_id);
    }
    else if (argc == 2)
    {
        char *endptr;
        long job_id_long = strtol(argv[1], &endptr, 10);
        if (*endptr != '\0' || job_id_long <= 0)
        {
            printf("No such job\n");
            return;
        }
        job = findJob((int)job_id_long);
    }
    else
    {
        printf("No such job\n");
        return;
    }

    if (!job)
    {
        printf("No such job\n");
        return;
    }

    if (job->state == JOB_RUNNING)
    {
        printf("Job already running\n");
        return;
    }

    // Only stopped jobs can be resumed with bg
    if (job->state != JOB_STOPPED)
    {
        printf("Job already running\n");
        return;
    }

    // Resume in background
    if (kill(-job->pgid, SIGCONT) == -1)
    {
        printf("No such job\n");
        return;
    }

    job->state = JOB_RUNNING;
    printf("[%d] %s &\n", job->job_id, extractCommandName(job->command));
}

// Compare function for qsort - sorts jobs by command name
static int compareJobsByCommand(const void *a, const void *b)
{
    const Job *jobA = (const Job *)a;
    const Job *jobB = (const Job *)b;

    char *cmdA = extractCommandName(jobA->command);
    char *cmdB = extractCommandName(jobB->command);

    return strcmp(cmdA, cmdB);
}
// ############## LLM Generated Code Ends ################
// E.1: activities command
void doActivities(void)
{
    Job active_jobs[MAX_JOBS];
    int active_count = 0;

    // First, update job states by checking which processes are still alive
    for (int i = 0; i < MAX_JOBS; i++)
    {
        if (jobs[i].active && jobs[i].state == JOB_RUNNING)
        {
            // Check if process is still running using kill with signal 0
            if (kill(jobs[i].pid, 0) == -1)
            {
                // Process no longer exists, but don't remove it yet - just mark as done
                jobs[i].state = JOB_DONE;
            }
        }
    }

    // Collect active jobs that are running or stopped (not done)
    for (int i = 0; i < MAX_JOBS; i++)
    {
        if (jobs[i].active && (jobs[i].state == JOB_RUNNING || jobs[i].state == JOB_STOPPED))
        {
            active_jobs[active_count++] = jobs[i];
        }
    }

    if (active_count == 0)
    {
        return; // No active jobs - print nothing
    }

    // Sort lexicographically by command name
    qsort(active_jobs, active_count, sizeof(Job), compareJobsByCommand);

    // Print jobs in the required format
    for (int i = 0; i < active_count; i++)
    {
        char *cmd_name = extractCommandName(active_jobs[i].command);
        char *state_str = (active_jobs[i].state == JOB_RUNNING) ? "Running" : "Stopped";
        printf("[%d] : %s - %s\n", active_jobs[i].pid, cmd_name, state_str);
    }
}
