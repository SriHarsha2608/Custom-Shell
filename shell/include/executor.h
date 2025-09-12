#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "tokenizer.h"

typedef struct
{
	char *command;
	char **args;
	int argc;
	char *input_file;
	char *output_file;
	int append_output;
	int redir_count;
	int redir_is_input[16];
	int redir_is_append[16];
	char *redir_path[16];
} Command;

typedef struct
{
	Command *commands;
	int command_count;
} Pipeline;

void execute_command_group(token *tokens, int count);
void execute_command_group_background(token *tokens, int count, char *original_command);
Pipeline parse_pipeline(token *tokens, int count);
void execute_pipeline(Pipeline *pipeline);
void execute_pipeline_background(Pipeline *pipeline, char *original_command);
void free_pipeline(Pipeline *pipeline);
int is_builtin(char *command);
void execute_builtin(char *command, char **args, int argc);

#endif