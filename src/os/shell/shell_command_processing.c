#include "../../includes.h"
#include "shell_internals.h"

/* ************************************************************************** */

int8_t find_matching_command(char *string) {
    for (uint8_t i = 0; i < MAXIMUM_NUM_OF_SHELL_COMMANDS; i++) {
        if (commands.list[i].callback == 0)
            continue;

        // If string matches one on the list
        if (!strcmp(string, commands.list[i].command)) {
            return i;
        }
    }
    return -1;
}

void process_shell_command(void) {
    int argc = 0;
    int length = strlen(shell.buffer) + 1;

    char *argv_list[CONFIG_SHELL_MAX_COMMAND_ARGS];

    argv_list[argc] = &shell.buffer[0];

    char currentChar = 0;
    char prevChar = 0;

    for (uint8_t i = 0; i < length; i++) {
        currentChar = shell.buffer[i];

        if (currentChar == '\0') {
            i = length;
            argc++;
        }
        if (currentChar == ' ') {
            shell.buffer[i] = '\0';
            argc++;
            argv_list[argc] = &shell.buffer[i + 1];
        }

        prevChar = shell.buffer[i];
    }

    int8_t command = find_matching_command(argv_list[0]);

    if (command != -1) {
        uint8_t retval = commands.list[command].callback(argc, argv_list);

        // process retval?

        return;
    }

    printf("%s: command not found\r\n", shell.buffer);
}