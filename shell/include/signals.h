#ifndef SIGNALS_H
#define SIGNALS_H

#include <sys/types.h>
#include <signal.h>

// Global variable for current foreground process group
extern pid_t current_fg_pgid;

// Global flag to indicate shell should exit immediately on EOF
extern volatile sig_atomic_t should_exit_on_eof;

// Self-pipe for EOF notification
extern int eof_pipe[2];

// Signal handlers
void sigint_handler(int sig);
void sigtstp_handler(int sig);
void sigquit_handler(int sig);

// Setup and utility functions
void setupSignalHandlers(void);
void setForegroundPgid(pid_t pgid);
void returnTerminalToShell(void);

#endif
