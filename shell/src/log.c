#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "log.h"
#include "tokenizer.h"
#include "parser.h"
#include "executor.h"

extern char shell_home[];

static char log_entries[MAX_LOG_ENTRIES][MAX_COMMAND_LENGTH];
static int log_count = 0;
static char log_file_path[1024];

void initLog(void)
{
    // ALWAYS reset counters first
    log_count = 0;

    // Create log file path in shell home directory
    snprintf(log_file_path, sizeof(log_file_path), "%s/.shell_log", shell_home);

    // Initialize log arrays - clear everything
    for (int i = 0; i < MAX_LOG_ENTRIES; i++)
    {
        log_entries[i][0] = '\0';
    }

    // Start fresh each session - clear any existing log file
    FILE *file = fopen(log_file_path, "w");
    if (file != NULL)
    {
        fclose(file); // Create empty file
    }

    // Log starts completely fresh for this session
}

void saveLogToFile(void)
{
    FILE *file = fopen(log_file_path, "w");
    if (file != NULL)
    {
        // Simple linear write - no complex circular buffer logic
        for (int i = 0; i < log_count; i++)
        {
            fprintf(file, "%s\n", log_entries[i]);
        }
        fflush(file); // Force write to disk
        fclose(file);
    }
}

int containsLogCommand(char *command)
{
    // Simple check - just look for "log" at the beginning
    return (strncmp(command, "log", 3) == 0 && (command[3] == ' ' || command[3] == '\0'));
}

void addToLog(char *command)
{
    // Skip null or empty commands
    if (command == NULL || strlen(command) == 0)
    {
        return;
    }

    // Don't store log commands
    if (containsLogCommand(command))
    {
        return;
    }

    // Don't store if identical to last command
    if (log_count > 0 && strcmp(log_entries[log_count - 1], command) == 0)
    {
        return;
    }

    // If buffer is full, shift everything left (remove oldest)
    if (log_count >= MAX_LOG_ENTRIES)
    {
        for (int i = 0; i < MAX_LOG_ENTRIES - 1; i++)
        {
            strcpy(log_entries[i], log_entries[i + 1]);
        }
        log_count = MAX_LOG_ENTRIES - 1;
    }

    // Add new command at the end
    strncpy(log_entries[log_count], command, MAX_COMMAND_LENGTH - 1);
    log_entries[log_count][MAX_COMMAND_LENGTH - 1] = '\0';
    log_count++;

    // Save immediately after each addition
    saveLogToFile();
}

void cleanupLog(void)
{
    // Optional: Clear log file when shell exits
    FILE *file = fopen(log_file_path, "w");
    if (file != NULL)
    {
        fclose(file); // Create empty file
    }

    // Clear in-memory log
    log_count = 0;
    for (int i = 0; i < MAX_LOG_ENTRIES; i++)
    {
        log_entries[i][0] = '\0';
    }
}

void doLog(int argc, char **argv)
{
    if (argc == 1)
    {
        // No arguments - print all commands (oldest to newest)
        if (log_count == 0)
        {
            return; // No output if no commands
        }

        // Print from oldest to newest
        for (int i = 0; i < log_count; i++)
        {
            printf("%s\n", log_entries[i]);
        }
    }
    else if (argc == 2 && strcmp(argv[1], "purge") == 0)
    {
        // Clear history
        log_count = 0;
        for (int i = 0; i < MAX_LOG_ENTRIES; i++)
        {
            log_entries[i][0] = '\0';
        }
        // Clear the file
        FILE *file = fopen(log_file_path, "w");
        if (file != NULL)
        {
            fclose(file);
        }
    }
    else if (argc == 3 && strcmp(argv[1], "execute") == 0)
    {
        // Execute command at index (1-indexed, newest to oldest)
        int index_input = atoi(argv[2]);
        if (index_input < 1 || index_input > log_count)
        {
            printf("Invalid index\n");
            return;
        }

        // Convert to array index (newest to oldest)
        int actual_index = log_count - index_input;
        char command_to_execute[MAX_COMMAND_LENGTH];
        strncpy(command_to_execute, log_entries[actual_index], MAX_COMMAND_LENGTH - 1);
        command_to_execute[MAX_COMMAND_LENGTH - 1] = '\0';

        // Parse and execute the command
        token tokens[256];
        int count;
        tokenize(command_to_execute, tokens, &count);

        if (parse(tokens))
        {
            execute_command_group(tokens, count);
        }
        else
        {
            printf("Invalid Syntax!\n");
        }
    }
    else
    {
        printf("log: Invalid Syntax!\n");
    }
}
