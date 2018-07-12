#include "../../includes.h"
#include "shell_internals.h"

/* ************************************************************************** */

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

void process_shell_command(void) {
    int argc = 0;
    char *argv_list[CONFIG_SHELL_MAX_COMMAND_ARGS];

    // tokenize the shell buffer
    // argv_list will end up containing a pointer to each token
    char *token = strtok(&shell.buffer, " ");
    while (token != NULL && argc <= CONFIG_SHELL_MAX_COMMAND_ARGS) {
        argv_list[argc++] = token;
        token = strtok(NULL, " ");
    }

    // figure out which command matches the received string
    int8_t command = find_matching_command(argv_list[0]);

    // if we found a valid command, execute it
    if (command != -1) {
        commands.list[command].callback(argc, argv_list);
        return;
    }
    // if there's no valid command, say something
    printf("%s: command not found\r\n", shell.buffer);
}