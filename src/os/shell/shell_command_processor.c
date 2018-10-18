#include "shell_command_processor.h"
#include "../../shell_commands.h"
#include "../console_io.h"
#include "shell_command_builtins.h"
#include <string.h>

/* ************************************************************************** */

const shell_command_t commandList[] = {
    BUILTIN_COMMANDS SHELL_COMMANDS{NULL, NULL},
};


/* -------------------------------------------------------------------------- */

// Print all registered shell commands
void shell_print_commands(void) {
    uint8_t i = 0;
    while (commandList[i].callback != NULL) {
        println(commandList[i].command);
    }
}

/* ************************************************************************** */

static int8_t find_matching_command(char *string) {
    for (uint8_t i = 0; i < MAXIMUM_NUM_OF_SHELL_COMMANDS; i++) {
        if (commandList[i].callback == 0)
            continue;

        // If string matches one on the list
        if (!strcmp(string, commandList[i].command)) {
            return i;
        }
    }
    return -1;
}

int argc;
char *argv_list[CONFIG_SHELL_MAX_COMMAND_ARGS];
int8_t command;

void process_shell_command(void) {
    argc = 0;

    // tokenize the shell buffer
    // argv_list will end up containing a pointer to each token
    char *token = strtok(&shell.buffer[0], " ");
    while (token != NULL && argc <= CONFIG_SHELL_MAX_COMMAND_ARGS) {
        argv_list[argc++] = token;
        token = strtok(NULL, " ");
    }

    // figure out which command matches the received string
    command = find_matching_command(argv_list[0]);

    // if we found a valid command, execute it
    if (command != -1) {
        shell_program_t program = commandList[command].callback;
        int result = program(argc, argv_list);

        if (result == 0) {
            print(SHELL_PROMPT_STRING);
        }
        return;
    }
    // if there's no valid command, say something
    printf("%s: command not found\r\n", shell.buffer);
    print(SHELL_PROMPT_STRING);
}