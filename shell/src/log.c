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
static int log_start = 0;  // Index of the oldest entry in circular buffer
static char log_file_path[1024];

void initLog(void) {
    // Create log file path in shell home directory
    snprintf(log_file_path, sizeof(log_file_path), "%s/.shell_log", shell_home);
    
    // Initialize log arrays
    for (int i = 0; i < MAX_LOG_ENTRIES; i++) {
        log_entries[i][0] = '\0';
    }
    
    // Load log from file
    FILE *file = fopen(log_file_path, "r");
    if (file != NULL) {
        char line[MAX_COMMAND_LENGTH];
        while (fgets(line, sizeof(line), file) != NULL && log_count < MAX_LOG_ENTRIES) {
            // Remove newline
            line[strcspn(line, "\n")] = '\0';
            if (strlen(line) > 0) {
                strcpy(log_entries[log_count], line);
                log_count++;
            }
        }
        fclose(file);
    }
}

void saveLogToFile(void) {
    FILE *file = fopen(log_file_path, "w");
    if (file != NULL) {
        // Write entries in chronological order (oldest to newest)
        for (int i = 0; i < log_count; i++) {
            int index = (log_start + i) % MAX_LOG_ENTRIES;
            fprintf(file, "%s\n", log_entries[index]);
        }
        fclose(file);
    }
}

int containsLogCommand(char *command) {
    // Tokenize the command to check if any atomic command is "log"
    token tokens[256];
    int count;
    tokenize(command, tokens, &count);
    
    for (int i = 0; i < count; i++) {
        if (tokens[i].type == T_NAME && strcmp(tokens[i].value, "log") == 0) {
            return 1;
        }
    }
    return 0;
}

void addToLog(char *command) {
    // Don't store if command contains log
    if (containsLogCommand(command)) {
        return;
    }
    
    // Don't store if identical to last command
    if (log_count > 0) {
        int last_index = (log_start + log_count - 1) % MAX_LOG_ENTRIES;
        if (strcmp(log_entries[last_index], command) == 0) {
            return;
        }
    }
    
    if (log_count < MAX_LOG_ENTRIES) {
        // Still have space
        strcpy(log_entries[log_count], command);
        log_count++;
    } else {
        // Buffer is full, overwrite oldest
        strcpy(log_entries[log_start], command);
        log_start = (log_start + 1) % MAX_LOG_ENTRIES;
    }
    
    saveLogToFile();
}

void doLog(int argc, char **argv) {
    if (argc == 1) {
        // No arguments - print all commands (oldest to newest)
        for (int i = 0; i < log_count; i++) {
            int index = (log_start + i) % MAX_LOG_ENTRIES;
            printf("%s\n", log_entries[index]);
        }
    } else if (argc == 2 && strcmp(argv[1], "purge") == 0) {
        // Clear history
        log_count = 0;
        log_start = 0;
        for (int i = 0; i < MAX_LOG_ENTRIES; i++) {
            log_entries[i][0] = '\0';
        }
        // Clear the file
        FILE *file = fopen(log_file_path, "w");
        if (file != NULL) {
            fclose(file);
        }
    } else if (argc == 3 && strcmp(argv[1], "execute") == 0) {
        // Execute command at index (1-indexed, newest to oldest)
        int index_input = atoi(argv[2]);
        if (index_input < 1 || index_input > log_count) {
            printf("Invalid index\n");
            return;
        }
        
        // Convert to our internal indexing (newest to oldest)
        int actual_index = (log_start + log_count - index_input) % MAX_LOG_ENTRIES;
        char command_to_execute[MAX_COMMAND_LENGTH];
        strcpy(command_to_execute, log_entries[actual_index]);
        
        // Parse and execute the command
        token tokens[256];
        int count;
        tokenize(command_to_execute, tokens, &count);
        
        if (parse(tokens)) {
            // Find the first command group (before ; or &)
            int cmd_group_end = 0;
            for (int i = 0; i < count && tokens[i].type != T_END; i++) {
                if (tokens[i].type == T_SEMI || tokens[i].type == T_AND) {
                    break;
                }
                cmd_group_end = i + 1;
            }
            
            // Execute the first command group
            execute_command_group(tokens, cmd_group_end);
        } else {
            printf("Invalid Syntax!\n");
        }
    } else {
        printf("log: Invalid Syntax!\n");
    }
}
