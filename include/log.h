#ifndef LOG_H
#define LOG_H

#define MAX_LOG_ENTRIES 15
#define MAX_COMMAND_LENGTH 1024

void initLog(void);
void addToLog(char *command);
void doLog(int argc, char **argv);
void cleanupLog(void);
int containsLogCommand(char *command);

#endif