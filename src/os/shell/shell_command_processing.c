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
    if (shell.length == 0) {
        return;
    }

    int argc = 0;
    int length = strlen(shell.buffer) + 1;
    char toggle = 0;

    char *argv_list[CONFIG_SHELL_MAX_COMMAND_ARGS];

    argv_list[argc] = &shell.buffer[0];

    /* //TODO: this wrongly parses multiple spaces in a command
        input "test 1 2 3"
        Received 4 arguments for test command
        0 - "test" [len:4]
        1 - "1" [len:1]
        2 - "2" [len:1]
        3 - "3" [len:1]

        input "test 1  2"
        Received 4 arguments for test command
        0 - "test" [len:4]
        1 - "1" [len:1]
        2 - "" [len:0] //* <- not correct
        3 - "2" [len:1]

        input: "test   "
        Received 4 arguments for test command
        0 - "test" [len:4]
        1 - "" [len:0] //* <- not correct
        2 - "" [len:0] //* <- not correct
        3 - "" [len:0] //* <- not correct
    */

    for (uint8_t i = 0; i < length && argc < CONFIG_SHELL_MAX_COMMAND_ARGS;
         i++) {
        switch (shell.buffer[i]) {
        case '\0': // String terminator means at least one arg
            i = length;
            argc++;
            break;
        case '\"': // Check for double quotes for strings as parameters
            if (toggle == 0) {
                toggle = 1;
                shell.buffer[i] = '\0';
                argv_list[argc] = &shell.buffer[i + 1];
            } else {
                toggle = 0;
                shell.buffer[i] = '\0';
            }
            break;
        case ' ': // Command arguments are separated by spaces
            if (toggle == 0) {
                shell.buffer[i] = '\0';
                argc++;
                argv_list[argc] = &shell.buffer[i + 1];
            }
            break;
        }
    }

    int8_t command = find_matching_command(argv_list[0]);

    if (command != -1) {
        uint8_t retval = commands.list[command].callback(argc, argv_list);

        // process retval?

        return;
    }

    printf("%s: command not found\r\n", shell.buffer);
}