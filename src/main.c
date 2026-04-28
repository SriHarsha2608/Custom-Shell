#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <limits.h>
#include "prompt.h"
#include "input.h"
#include "parser.h"
#include "tokenizer.h"
#include "hop.h"
#include "reveal.h"
#include "executor.h"
#include "log.h"
#include "jobs.h"
#include "signals.h"

char shell_home[PATH_MAX];

int main()
{
    if (getcwd(shell_home, sizeof(shell_home)) == NULL)
    {
        perror("getcwd");
        exit(EXIT_FAILURE);
    }
    initHop();
    initLog();
    initJobs();

    // E.3: Setup signal handlers for job control
    setupSignalHandlers();

    char input[1024];
    token tokens[256];
    int count;

    while (1)
    {
        display_prompt();
        int len = userInput(input, sizeof(input));

        // Check for completed background processes after user input, before parsing
        checkBackgroundJobs();

        // E.3: Handle EOF (Ctrl-D)
        if (len == 0)
        {
            printf("logout\n");
            cleanupLog();
            exit(0);
        }

        if (len <= 0)
            continue;

        tokenize(input, tokens, &count);

        if (!parse(tokens))
        {
            printf("Invalid Syntax!\n");
            continue;
        }

        if (count == 0 || tokens[0].type == T_END)
        {
            continue;
        }

        // Check if it's a log command (handle separately)
        if (tokens[0].type == T_NAME && strcmp(tokens[0].value, "log") == 0)
        {
            int argc = 0;
            char *argv[32];
            for (int i = 0; i < count && tokens[i].type != T_END; i++)
            {
                if (tokens[i].type == T_NAME)
                {
                    argv[argc++] = tokens[i].value;
                }
            }
            doLog(argc, argv);
            continue;
        }

        // Add command to log before executing
        addToLog(input);

        // Parse and execute sequential and background commands
        int i = 0;
        while (i < count && tokens[i].type != T_END)
        {
            // Find the end of current command group
            int cmd_start = i;
            int cmd_end = i;

            // Find the end of this command group (before ; or &)
            while (cmd_end < count && tokens[cmd_end].type != T_END &&
                   tokens[cmd_end].type != T_SEMI && tokens[cmd_end].type != T_AND)
            {
                cmd_end++;
            }

            // Check if this command group should run in background
            int is_background = 0;
            if (cmd_end < count && tokens[cmd_end].type == T_AND)
            {
                is_background = 1;
            }

            // Create command string for background jobs
            char cmd_str[1024];
            if (is_background)
            {
                cmd_str[0] = '\0';
                for (int j = cmd_start; j < cmd_end && tokens[j].type != T_END; j++)
                {
                    if (j > cmd_start)
                    {
                        strncat(cmd_str, " ", sizeof(cmd_str) - strlen(cmd_str) - 1);
                    }
                    strncat(cmd_str, tokens[j].value, sizeof(cmd_str) - strlen(cmd_str) - 1);
                }
            }

            // Execute the command group
            if (is_background)
            {
                execute_command_group_background(tokens + cmd_start, cmd_end - cmd_start, cmd_str);
            }
            else
            {
                execute_command_group(tokens + cmd_start, cmd_end - cmd_start);
            }

            // Move to next command group
            if (cmd_end < count && (tokens[cmd_end].type == T_SEMI || tokens[cmd_end].type == T_AND))
            {
                i = cmd_end + 1;
            }
            else
            {
                break;
            }
        }
    }

    cleanupLog();
    return 0;
}
