#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <termios.h>
#include "signals.h"
#include "jobs.h"
#include "prompt.h"

// Global variable to track current foreground process group
pid_t current_fg_pgid = 0;

// Global flag to indicate shell should exit immediately on EOF
volatile sig_atomic_t should_exit_on_eof = 0;

// ############## LLM Generated Code Begins ##############
void sigint_handler(int sig)
{
    (void)sig; // Suppress unused parameter warning

    // If EOF was detected, exit immediately
    if (should_exit_on_eof)
    {
        write(STDOUT_FILENO, "logout\n", 7);
        _exit(0);
    }

    // If there's a foreground job, send SIGINT to its process group
    if (current_fg_pgid > 0 && current_fg_pgid != getpgrp())
    {
        kill(-current_fg_pgid, SIGINT);
    }

    // Print newline to move to next line after ^C
    write(STDOUT_FILENO, "\n", 1);
}

// E.3: Ctrl-Z handler (SIGTSTP)
void sigtstp_handler(int sig)
{
    (void)sig; // Suppress unused parameter warning

    // If EOF was detected, exit immediately
    if (should_exit_on_eof)
    {
        write(STDOUT_FILENO, "logout\n", 7);
        _exit(0);
    }

    // If there's a foreground job, send SIGTSTP to its process group
    if (current_fg_pgid > 0 && current_fg_pgid != getpgrp())
    {
        kill(-current_fg_pgid, SIGTSTP);

        // Find the job and mark it as stopped
        Job *job = findJobByPid(current_fg_pgid);
        if (job)
        {
            job->state = JOB_STOPPED;

            // Extract command name (first word)
            char *cmd_name = extractCommandName(job->command);

            // Print stopped message
            char msg[512];
            int n = snprintf(msg, sizeof(msg), "\n[%d] Stopped %s\n", job->job_id, cmd_name);
            if (n > 0)
            {
                write(STDOUT_FILENO, msg, (size_t)n);
            }
        }

        // Return terminal control to shell and clear foreground process group
        returnTerminalToShell();
        current_fg_pgid = 0;

        // Immediately show a fresh prompt
        display_prompt();
    }
}

// Install signal handlers for job control
void setupSignalHandlers(void)
{
    struct sigaction sa;

    // Install SIGINT handler (Ctrl-C)
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGINT, &sa, NULL) == -1)
    {
        perror("sigaction SIGINT");
        exit(EXIT_FAILURE);
    }

    // Install SIGTSTP handler (Ctrl-Z)
    sa.sa_handler = sigtstp_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGTSTP, &sa, NULL) == -1)
    {
        perror("sigaction SIGTSTP");
        exit(EXIT_FAILURE);
    }

    // Install SIGQUIT handler for Ctrl+D immediate exit
    sa.sa_handler = sigquit_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0; // No SA_RESTART for immediate response
    if (sigaction(SIGQUIT, &sa, NULL) == -1)
    {
        perror("sigaction SIGQUIT");
        exit(EXIT_FAILURE);
    }

    // Ignore SIGTTOU so we can call tcsetpgrp
    signal(SIGTTOU, SIG_IGN);
}

// Set foreground process group
void setForegroundPgid(pid_t pgid)
{
    current_fg_pgid = pgid;
}

// SIGQUIT handler for Ctrl+D immediate exit
void sigquit_handler(int sig)
{
    (void)sig; // Suppress unused parameter warning

    // If there's a foreground process running, send EOF to it, don't exit shell
    if (current_fg_pgid > 0 && current_fg_pgid != getpgrp())
    {
        // Send SIGTERM to foreground process to simulate EOF
        kill(-current_fg_pgid, SIGTERM);
        return; // Don't exit the shell, let the process handle EOF
    }

    // Only exit shell if no foreground process is running (i.e., at shell prompt)
    write(STDOUT_FILENO, "logout\n", 7);
    _exit(0);
}

// Return terminal control to shell
void returnTerminalToShell(void)
{
    current_fg_pgid = 0;
    // No need to call tcsetpgrp since we never gave control away
}
// ############## LLM Generated Code Ends ##############