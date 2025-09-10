#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include "tokenizer.h"
#include "executor.h"
#include "hop.h"
#include "reveal.h"
#include "log.h"
#include "jobs.h"
#include "ping.h"

int is_builtin(char *command) {
    return (strcmp(command, "hop") == 0 || 
            strcmp(command, "reveal") == 0 ||
            strcmp(command, "log") == 0 ||
            strcmp(command, "activities") == 0 ||
            strcmp(command, "ping") == 0);
}

void execute_builtin(char *command, char **args, int argc) {
    if (strcmp(command, "hop") == 0) {
        doHop(argc, args);
    } else if (strcmp(command, "reveal") == 0) {
        doReveal(argc, args);
    } else if (strcmp(command, "log") == 0) {
        doLog(argc, args);
    } else if (strcmp(command, "activities") == 0) {
        doActivities();
    } else if (strcmp(command, "ping") == 0) {
        doPing(argc, args);
    }
}

Pipeline parse_pipeline(token *tokens, int count) {
    Pipeline pipeline;
    memset(&pipeline, 0, sizeof(Pipeline));
    pipeline.commands = NULL;
    pipeline.command_count = 0;
    
    if (count <= 0) {
        return pipeline;
    }
    
    int capacity = 4;
    pipeline.commands = malloc(capacity * sizeof(Command));
    if (pipeline.commands == NULL) {
        perror("malloc");
        return pipeline;
    }
    
    int i = 0;
    while (i < count && tokens[i].type != T_END) {
        // Resize if needed
        if (pipeline.command_count >= capacity) {
            capacity *= 2;
            Command *temp = realloc(pipeline.commands, capacity * sizeof(Command));
            if (temp == NULL) {
                free_pipeline(&pipeline);
                perror("realloc");
                return pipeline;
            }
            pipeline.commands = temp;
        }
        
        Command *cmd = &pipeline.commands[pipeline.command_count];
        memset(cmd, 0, sizeof(Command));
        cmd->command = NULL;
        cmd->args = NULL;
        cmd->argc = 0;
        cmd->input_file = NULL;
        cmd->output_file = NULL;
        cmd->append_output = 0;
        
        // Parse command and arguments
        int arg_capacity = 16;
        cmd->args = malloc(arg_capacity * sizeof(char*));
        if (cmd->args == NULL) {
            perror("malloc");
            free_pipeline(&pipeline);
            return pipeline;
        }
        
        // First token should be the command name
        if (tokens[i].type == T_NAME) {
            cmd->command = strdup(tokens[i].value);
            cmd->args[cmd->argc++] = strdup(tokens[i].value);
            i++;
        }
        
        // Parse arguments and redirections
        while (i < count && tokens[i].type != T_PIPE && 
               tokens[i].type != T_END && tokens[i].type != T_SEMI && 
               tokens[i].type != T_AND) {
            
            if (tokens[i].type == T_NAME) {
                if (cmd->argc >= arg_capacity - 1) {
                    arg_capacity *= 2;
                    char **temp = realloc(cmd->args, arg_capacity * sizeof(char*));
                    if (temp == NULL) {
                        perror("realloc");
                        free_pipeline(&pipeline);
                        return pipeline;
                    }
                    cmd->args = temp;
                }
                cmd->args[cmd->argc++] = strdup(tokens[i].value);
                i++;
            } else if (tokens[i].type == T_INPUT) {
                i++; // Skip the '<'
                if (i < count && tokens[i].type == T_NAME) {
                    if (cmd->input_file) free(cmd->input_file);
                    cmd->input_file = strdup(tokens[i].value);
                    i++;
                }
            } else if (tokens[i].type == T_OUTPUT) {
                i++; // Skip the '>'
                if (i < count && tokens[i].type == T_NAME) {
                    if (cmd->output_file) free(cmd->output_file);
                    cmd->output_file = strdup(tokens[i].value);
                    cmd->append_output = 0;
                    i++;
                }
            } else if (tokens[i].type == T_APPEND) {
                i++; // Skip the '>>'
                if (i < count && tokens[i].type == T_NAME) {
                    if (cmd->output_file) free(cmd->output_file);
                    cmd->output_file = strdup(tokens[i].value);
                    cmd->append_output = 1;
                    i++;
                }
            } else {
                i++;
            }
        }
        
        cmd->args[cmd->argc] = NULL; // NULL terminate
        pipeline.command_count++;
        
        // Skip pipe token
        if (i < count && tokens[i].type == T_PIPE) {
            i++;
        } else {
            break;
        }
    }
    
    return pipeline;
}

void setup_redirection(Command *cmd) {
    if (cmd->input_file) {
        int fd = open(cmd->input_file, O_RDONLY);
        if (fd == -1) {
            fprintf(stderr, "No such file or directory\n");
            exit(1);
        }
        if (dup2(fd, STDIN_FILENO) == -1) {
            perror("dup2");
            close(fd);
            exit(1);
        }
        close(fd);
    }
    
    if (cmd->output_file) {
        int fd;
        if (cmd->append_output) {
            fd = open(cmd->output_file, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        } else {
            fd = open(cmd->output_file, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        }
        if (fd == -1) {
            fprintf(stderr, "No such file or directory\n");
            exit(1);
        }
        if (dup2(fd, STDOUT_FILENO) == -1) {
            perror("dup2");
            close(fd);
            exit(1);
        }
        close(fd);
    }
}

void execute_pipeline(Pipeline *pipeline) {
    if (pipeline->command_count == 0) return;
    
    // Single command case
    if (pipeline->command_count == 1) {
        Command *cmd = &pipeline->commands[0];
        
        if (is_builtin(cmd->command)) {
            // Handle redirection for builtins
            int saved_stdin = -1, saved_stdout = -1;
            
            if (cmd->input_file || cmd->output_file) {
                saved_stdin = dup(STDIN_FILENO);
                if (saved_stdin == -1) {
                    perror("dup");
                    return;
                }
                saved_stdout = dup(STDOUT_FILENO);
                if (saved_stdout == -1) {
                    perror("dup");
                    if (saved_stdin != -1) close(saved_stdin);
                    return;
                }
                setup_redirection(cmd);
            }
            
            execute_builtin(cmd->command, cmd->args, cmd->argc);
            
            // Restore original stdin/stdout
            if (saved_stdin != -1) {
                dup2(saved_stdin, STDIN_FILENO);
                close(saved_stdin);
            }
            if (saved_stdout != -1) {
                dup2(saved_stdout, STDOUT_FILENO);
                close(saved_stdout);
            }
        } else {
            pid_t pid = fork();
            if (pid == 0) {
                setup_redirection(cmd);
                execvp(cmd->command, cmd->args);
                perror("execvp");
                exit(127);
            } else if (pid > 0) {
                int status;
                if (waitpid(pid, &status, 0) == -1) {
                    perror("waitpid");
                }
            } else {
                perror("fork");
            }
        }
        return;
    }
    
    // Multiple commands - pipeline
    int pipes[pipeline->command_count - 1][2];
    pid_t pids[pipeline->command_count];
    
    // Create all pipes
    for (int i = 0; i < pipeline->command_count - 1; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            return;
        }
    }
    
    // Execute each command
    for (int i = 0; i < pipeline->command_count; i++) {
        Command *cmd = &pipeline->commands[i];
        
        pids[i] = fork();
        if (pids[i] == -1) {
            perror("fork");
            // Clean up pipes and exit
            for (int j = 0; j < pipeline->command_count - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            return;
        }
        if (pids[i] == 0) {
            // Setup pipes
            if (i > 0) {
                dup2(pipes[i-1][0], STDIN_FILENO);
            }
            if (i < pipeline->command_count - 1) {
                dup2(pipes[i][1], STDOUT_FILENO);
            }
            
            // Close all pipe fds
            for (int j = 0; j < pipeline->command_count - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            
            setup_redirection(cmd);
            
            if (is_builtin(cmd->command)) {
                execute_builtin(cmd->command, cmd->args, cmd->argc);
                exit(0);
            } else {
                execvp(cmd->command, cmd->args);
                perror("execvp");
                exit(127);
            }
        }
    }
    
    // Close all pipe fds in parent
    for (int i = 0; i < pipeline->command_count - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
    
    // Wait for all children
    for (int i = 0; i < pipeline->command_count; i++) {
        if (pids[i] > 0) {
            int status;
            if (waitpid(pids[i], &status, 0) == -1) {
                perror("waitpid");
            }
        }
    }
}

void execute_pipeline_background(Pipeline *pipeline, char *original_command) {
    if (pipeline->command_count == 0) return;
    
    // For single commands, fork directly to avoid double-fork issue
    if (pipeline->command_count == 1) {
        Command *cmd = &pipeline->commands[0];
        
        pid_t pid = fork();
        if (pid == 0) {
            // Child process - redirect stdin to /dev/null for background processes
            int null_fd = open("/dev/null", O_RDONLY);
            if (null_fd == -1) {
                perror("open /dev/null");
                exit(EXIT_FAILURE);
            }
            if (dup2(null_fd, STDIN_FILENO) == -1) {
                perror("dup2");
                close(null_fd);
                exit(EXIT_FAILURE);
            }
            close(null_fd);
            
            setup_redirection(cmd);
            
            if (is_builtin(cmd->command)) {
                execute_builtin(cmd->command, cmd->args, cmd->argc);
                exit(0);
            } else {
                execvp(cmd->command, cmd->args);
                perror("execvp");
                exit(127);
            }
        } else if (pid > 0) {
            // Parent process - add to job list
            addJob(pid, original_command);
        } else {
            perror("fork");
        }
    } else {
        // For pipelines, use the original double-fork approach
        pid_t main_pid = fork();
        if (main_pid == 0) {
            // Child process - redirect stdin to /dev/null for background processes
            int null_fd = open("/dev/null", O_RDONLY);
            if (null_fd == -1) {
                perror("open /dev/null");
                exit(EXIT_FAILURE);
            }
            if (dup2(null_fd, STDIN_FILENO) == -1) {
                perror("dup2");
                close(null_fd);
                exit(EXIT_FAILURE);
            }
            close(null_fd);
            
            // Execute the pipeline normally
            execute_pipeline(pipeline);
            exit(0);
        } else if (main_pid > 0) {
            // Parent process - add to job list
            addJob(main_pid, original_command);
        } else {
            perror("fork");
        }
    }
}

void execute_command_group(token *tokens, int count) {
    Pipeline pipeline = parse_pipeline(tokens, count);
    execute_pipeline(&pipeline);
    free_pipeline(&pipeline);
}

void execute_command_group_background(token *tokens, int count, char *original_command) {
    Pipeline pipeline = parse_pipeline(tokens, count);
    execute_pipeline_background(&pipeline, original_command);
    free_pipeline(&pipeline);
}

void free_pipeline(Pipeline *pipeline) {
    for (int i = 0; i < pipeline->command_count; i++) {
        Command *cmd = &pipeline->commands[i];
        if (cmd->command) free(cmd->command);
        if (cmd->input_file) free(cmd->input_file);
        if (cmd->output_file) free(cmd->output_file);
        if (cmd->args) {
            for (int j = 0; j < cmd->argc; j++) {
                if (cmd->args[j]) free(cmd->args[j]);
            }
            free(cmd->args);
        }
    }
    if (pipeline->commands) free(pipeline->commands);
    pipeline->commands = NULL;
    pipeline->command_count = 0;
}
