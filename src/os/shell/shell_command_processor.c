#include "shell_command_processor.h"
#include "../../shell_commands.h"
#include "../console_io.h"
#include <string.h>

/* ************************************************************************** */
// forward declaration
extern void shell_print_commands(void);

int shell_help(int argc, char **argv) {
    println("-----------------------------------------------");
    shell_print_commands();
    println("-----------------------------------------------");

    return 0;
}

const char argTestUsage[] = "\
This command has no special arguments.\r\n\
It is designed to test the TuneOS shell's arg parsing.\r\n\
\r\n\
Use it like this:\r\n\
\"$ test command arg1 arg2 arg3\"\r\n\
\r\n\
To get this response:\r\n\
Received 4 arguments for test command\r\n\
1 - \"command\" [len:7]\r\n\
2 - \"arg1\" [len:4]\r\n\
3 - \"arg2\" [len:4]\r\n\
4 - \"arg3\" [len:4]\r\n\
";

int shell_arg_test(int argc, char **argv) {
    println("-----------------------------------------------");
    println("SHELL ARG PARSING TEST UTILITY");
    if (argc == 1) {
        print(argTestUsage);
    } else {
        printf("Received %d arguments for test command\r\n", argc - 1);

        // Prints: <argNum> - "<string>" [len:<length>]
        for (uint8_t i = 1; i < argc; i++) {
            printf("%d - \"%s\" [len:%d]\r\n", i, argv[i], strlen(argv[i]));
        }
    }
    println("-----------------------------------------------");

    return 0;
}

#define BUILTIN_SHELL_COMMANDS                                                 \
    {shell_help, "help"}, { shell_arg_test, "test" }
/* ************************************************************************** */

/*  shell_command_t

    callback
    A pointer to the shell command body.

    command
    A pointer to string that represents the command that needs to be typed
*/
typedef struct {
    shell_program_t callback;
    const char *command;
} shell_command_t;

const shell_command_t commandList[] = {
    BUILTIN_SHELL_COMMANDS,
    SHELL_COMMANDS,
    {NULL, NULL},
};

/* -------------------------------------------------------------------------- */

// Print all registered shell commands
void shell_print_commands(void) {
    uint8_t i = 0;
    while (commandList[i].callback != NULL) {
        println(commandList[i++].command);
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

void process_shell_command(void) {
    int argc = 0;
    char *argv_list[CONFIG_SHELL_MAX_COMMAND_ARGS];

    // tokenize the shell buffer
    // argv_list will end up containing a pointer to each token
    char *token = strtok(&shell.buffer[0], " ");
    while (token != NULL && argc <= CONFIG_SHELL_MAX_COMMAND_ARGS) {
        argv_list[argc++] = token;
        token = strtok(NULL, " ");
    }

    // figure out which command matches the received string
    int8_t command = find_matching_command(argv_list[0]);

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