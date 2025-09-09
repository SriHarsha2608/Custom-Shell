// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include "prompt.h"
// #include "input.h"
// #include "parser.h"
// #include "tokenizer.h"
// #include "hop.h"
// #include "reveal.h"
// #include "executor.h"
// #include "log.h"
// #include "jobs.h"

// char shell_home[1024];

// // Helper function to extract command group from tokens
// void extractCommandGroup(token *tokens, int start, int end, char *output) {
//     output[0] = '\0';
//     for (int i = start; i < end && tokens[i].type != T_END; i++) {
//         if (i > start) strcat(output, " ");
//         strcat(output, tokens[i].value);
//     }
// }

// int main() {
//     getcwd(shell_home, sizeof(shell_home));
//     initHop();
//     initLog();
//     initJobs();
    
//     char input[1024];
//     token tokens[256];
//     int count;
    
//     while (1) {
//         // Check for completed background jobs before displaying prompt
//         checkBackgroundJobs();
        
//         display_prompt();
//         int len = userInput(input, sizeof(input));
        
//         if (len <= 0) continue;
        
//         tokenize(input, tokens, &count);
        
//         if (!parse(tokens)) {
//             printf("Invalid Syntax!\n");
//             continue;
//         }
        
//         if (count == 0 || tokens[0].type == T_END) {
//             continue;
//         }
        
//         // Check if it's a log command (handle separately)
//         if (tokens[0].type == T_NAME && strcmp(tokens[0].value, "log") == 0) {
//             int argc = 0;
//             char *argv[32];
//             for (int i = 0; i < count && tokens[i].type != T_END; i++) {
//                 if (tokens[i].type == T_NAME) {
//                     argv[argc++] = tokens[i].value;
//                 }
//             }
//             doLog(argc, argv);
//             continue;
//         }
        
//         // Add command to log before executing
//         addToLog(input);
        
//         // Parse the entire shell command for sequential and background execution
//         int i = 0;
//         while (i < count && tokens[i].type != T_END) {
//             // Find the end of current command group
//             int cmd_start = i;
//             int cmd_end = i;
            
//             // Find the end of this command group (before ; or &)
//             while (cmd_end < count && tokens[cmd_end].type != T_END &&
//                    tokens[cmd_end].type != T_SEMI && tokens[cmd_end].type != T_AND) {
//                 cmd_end++;
//             }
            
//             // Check if this command group should run in background
//             int is_background = 0;
//             if (cmd_end < count && tokens[cmd_end].type == T_AND) {
//                 is_background = 1;
                
//                 // Check if there are more commands after &
//                 int next_cmd = cmd_end + 1;
//                 while (next_cmd < count && tokens[next_cmd].type == T_END) {
//                     next_cmd++;
//                 }
                
//                 // If there are more commands after &, only the current one runs in background
//                 if (next_cmd < count && tokens[next_cmd].type != T_END) {
//                     is_background = 1; // Only current command runs in background
//                 }
//             }
            
//             // Execute the command group
//             if (is_background) {
//                 char cmd_str[1024];
//                 extractCommandGroup(tokens, cmd_start, cmd_end, cmd_str);
//                 execute_command_group_background(tokens + cmd_start, cmd_end - cmd_start, cmd_str);
//             } else {
//                 execute_command_group(tokens + cmd_start, cmd_end - cmd_start);
//             }
            
//             // Move to next command group
//             if (cmd_end < count && (tokens[cmd_end].type == T_SEMI || tokens[cmd_end].type == T_AND)) {
//                 i = cmd_end + 1;
                
//                 // For background execution with more commands, only first runs in background
//                 if (tokens[cmd_end].type == T_AND && i < count && tokens[i].type != T_END) {
//                     // Continue with sequential execution for remaining commands
//                     continue;
//                 } else if (tokens[cmd_end].type == T_AND) {
//                     // End of command if & is at the end
//                     break;
//                 }
//             } else {
//                 break;
//             }
//         }
//     }
    
//     return 0;
// }

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "prompt.h"
#include "input.h"
#include "parser.h"
#include "tokenizer.h"
#include "hop.h"
#include "reveal.h"
#include "executor.h"
#include "log.h"
#include "jobs.h"

char shell_home[1024];
volatile sig_atomic_t child_exited = 0;

// Signal handler for SIGCHLD
void sigchld_handler(int sig) {
    (void)sig; // Suppress unused parameter warning
    child_exited = 1;
}

// Helper function to extract command group from tokens
void extractCommandGroup(token *tokens, int start, int end, char *output) {
    output[0] = '\0';
    for (int i = start; i < end && tokens[i].type != T_END; i++) {
        if (i > start) strcat(output, " ");
        strcat(output, tokens[i].value);
    }
}

int main() {
    getcwd(shell_home, sizeof(shell_home));
    initHop();
    initLog();
    initJobs();
    
    // Install SIGCHLD handler for immediate background job notifications
    signal(SIGCHLD, sigchld_handler);
    
    char input[1024];
    token tokens[256];
    int count;
    
    while (1) {
        // Check if any child exited (signal handler sets this flag)
        if (child_exited) {
            checkBackgroundJobs();
            child_exited = 0;
        }
        
        // Check for completed background jobs before displaying prompt
        checkBackgroundJobs();
        
        display_prompt();
        int len = userInput(input, sizeof(input));
        
        // Check again after user input in case jobs completed while typing
        checkBackgroundJobs();
        
        if (len <= 0) continue;
        
        tokenize(input, tokens, &count);
        
        if (!parse(tokens)) {
            printf("Invalid Syntax!\n");
            continue;
        }
        
        if (count == 0 || tokens[0].type == T_END) {
            continue;
        }
        
        // Check if it's a log command (handle separately)
        if (tokens[0].type == T_NAME && strcmp(tokens[0].value, "log") == 0) {
            int argc = 0;
            char *argv[32];
            for (int i = 0; i < count && tokens[i].type != T_END; i++) {
                if (tokens[i].type == T_NAME) {
                    argv[argc++] = tokens[i].value;
                }
            }
            doLog(argc, argv);
            continue;
        }
        
        // Add command to log before executing
        addToLog(input);
        
        // Parse the entire shell command for sequential and background execution
        int i = 0;
        while (i < count && tokens[i].type != T_END) {
            // Find the end of current command group
            int cmd_start = i;
            int cmd_end = i;
            
            // Find the end of this command group (before ; or &)
            while (cmd_end < count && tokens[cmd_end].type != T_END &&
                   tokens[cmd_end].type != T_SEMI && tokens[cmd_end].type != T_AND) {
                cmd_end++;
            }
            
            // Check if this command group should run in background
            int is_background = 0;
            if (cmd_end < count && tokens[cmd_end].type == T_AND) {
                is_background = 1;
                
                // Check if there are more commands after &
                int next_cmd = cmd_end + 1;
                while (next_cmd < count && tokens[next_cmd].type == T_END) {
                    next_cmd++;
                }
                
                // If there are more commands after &, only the current one runs in background
                if (next_cmd < count && tokens[next_cmd].type != T_END) {
                    is_background = 1; // Only current command runs in background
                }
            }
            
            // Execute the command group
            if (is_background) {
                char cmd_str[1024];
                extractCommandGroup(tokens, cmd_start, cmd_end, cmd_str);
                execute_command_group_background(tokens + cmd_start, cmd_end - cmd_start, cmd_str);
            } else {
                execute_command_group(tokens + cmd_start, cmd_end - cmd_start);
            }
            
            // Move to next command group
            if (cmd_end < count && (tokens[cmd_end].type == T_SEMI || tokens[cmd_end].type == T_AND)) {
                i = cmd_end + 1;
                
                // For background execution with more commands, only first runs in background
                if (tokens[cmd_end].type == T_AND && i < count && tokens[i].type != T_END) {
                    // Continue with sequential execution for remaining commands
                    continue;
                } else if (tokens[cmd_end].type == T_AND) {
                    // End of command if & is at the end
                    break;
                }
            } else {
                break;
            }
        }
    }
    
    return 0;
}
