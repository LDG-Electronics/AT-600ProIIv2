#include "shell_command_processor.h"
#include "../../shell_commands.h"
#include "../serial_port.h"
#include "shell.h"
#include "shell_config.h"
#include <string.h>

/* ************************************************************************** */
// forward declaration
extern void shell_print_commands(void);

/* ************************************************************************** */

// prints all registered commands
void shell_help(int argc, char **argv) {
    println("-----------------------------------------------");
    shell_print_commands();
    println("-----------------------------------------------");
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

void shell_arg_test(int argc, char **argv) {
    println("-----------------------------------------------");
    println("SHELL ARG PARSING TEST UTILITY");
    if (argc == 1) {
        print(argTestUsage);
    } else {
        printf("Received %d arguments for test command\r\n", argc - 1);

        // Prints: <argNum> - "<string>" [len:<length>]
        for (uint8_t i = 1; i < argc; i++) {
            printf("%u - \"%s\" [len:%u]\r\n", i, argv[i], strlen(argv[i]));
        }
    }
    println("-----------------------------------------------");
}

#define BUILTIN_SHELL_COMMANDS                                                 \
    {shell_help, "help"}, { shell_arg_test, "test" }

/* ************************************************************************** */

/*  shell_command_t

    program
    A pointer to the shell command body.

    command
    A pointer to string that represents the command that needs to be typed
*/
typedef struct {
    shell_program_t program;
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
    while (commandList[i].program != NULL) {
        println(commandList[i++].command);
    }
}

/* ************************************************************************** */

// returns the index of the command that matches *string
static int8_t find_matching_command(char *string) {
    uint8_t i = 0;
    while (1) {
        // commandList is NULL-terminated, so break on NULL
        if (!commandList[i].program) {
            break;
        }

        // If string matches one on the list
        if (!strcmp(string, commandList[i].command)) {
            return i;
        }

        i++;
    }
    return -1;
}

int8_t process_shell_command(char *string) {
    int argc = 0;
    char *argv_list[CONFIG_SHELL_MAX_COMMAND_ARGS];

    // tokenize the shell buffer
    // argv_list will end up containing a pointer to each token
    char *token = strtok(string, " ");
    while (token != NULL && argc <= CONFIG_SHELL_MAX_COMMAND_ARGS) {
        argv_list[argc++] = token;
        token = strtok(NULL, " ");
    }

    // figure out which command matches the received string
    int8_t command = find_matching_command(argv_list[0]);

    // if we found a valid command, execute it
    if (command != -1) {
        commandList[command].program(argc, argv_list);

        return 0;
    }
    return -1;
}