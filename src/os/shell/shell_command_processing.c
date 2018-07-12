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

    int8_t command = find_matching_command(argv_list[0]);

    if (command != -1) {
        uint8_t retval = commands.list[command].callback(argc, argv_list);

        // process retval?

        return;
    }

    printf("%s: command not found\r\n", shell.buffer);
}