#include "shell_command_processor.h"

/* ************************************************************************** */

/*  shell command list

    This data structure is the central registry for shell commands.

    Individual commands should be defined in shell_commands.c and registered in
    shell_commands_init().

    The function signature of a shell command must be:
    int (*shell_program_t) (int, char **)
*/

/*  shell_command_t

    callback
    A pointer to the shell command body.

    command
    A pointer to string that represents the command that needs to be typed
*/
typedef struct {
    shell_program_t callback;
    const char *command;
    const char *usage;
} shell_command_t;

/*  command_list_t

    list
    An array of shell_command_t objects that stores all the registered shell
    commands.

    numOfRegisteredCommands
    The number of commands registered with the shell
*/
typedef struct commands {
    shell_command_t list[MAXIMUM_NUM_OF_SHELL_COMMANDS];
    uint8_t number;
} command_list_t;

command_list_t commands;

void clear_shell_command(uint8_t index) {
    commands.list[index].callback = 0;
    commands.list[index].command = 0;
    commands.list[index].usage = 0;
}

/* -------------------------------------------------------------------------- */

// Print all registered shell commands
void shell_print_commands(void) {
    for (uint8_t i = 0; i < commands.number; i++) {
        if (commands.list[i].callback != 0 || commands.list[i].command != 0) {
            println(commands.list[i].command);
        }
    }
}

/* ************************************************************************** */

void shell_commands_init(void) {
    // clear the entire command list
    for (uint8_t i = 0; i < MAXIMUM_NUM_OF_SHELL_COMMANDS; i++) {
        clear_shell_command(i);
    }
    commands.number = 0;
}

// Add a command to the command list
void shell_register_command(shell_program_t program, const char *command,
                            const char *usage) {
    commands.list[commands.number].callback = program;
    commands.list[commands.number].command = command;
    commands.list[commands.number].usage = usage;

    commands.number++;
}

/* -------------------------------------------------------------------------- */

static int8_t find_matching_command(char *string) {
    for (uint8_t i = 0; i < MAXIMUM_NUM_OF_SHELL_COMMANDS; i++) {
        if (commands.list[i].callback == 0)
            continue;

        // If string matches one on the list
        if (!stricmp(string, commands.list[i].command)) {
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
    char *token = strtok(&shell.buffer, " ");
    while (token != NULL && argc <= CONFIG_SHELL_MAX_COMMAND_ARGS) {
        argv_list[argc++] = token;
        token = strtok(NULL, " ");
    }

    // figure out which command matches the received string
    command = find_matching_command(argv_list[0]);

    // if we found a valid command, execute it
    if (command != -1) {
        shell_program_t program = commands.list[command].callback;
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

void execute_shell_command(void) {}