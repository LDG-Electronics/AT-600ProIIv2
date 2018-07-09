#include "../../includes.h"
#include "shell_internals.h"

/* ************************************************************************** */

shell_t shell;

/* ************************************************************************** */

// reset shell.buffer to 0
void reset_current_line(void) {
    memset(&shell.buffer[0], NULL, SHELL_MAX_LENGTH);

    shell.length = 0;
    shell.cursor = 0;
}

// reset all shell state flags to default values
void reset_shell_flags(void) {
    shell.escapeMode = 0;
    shell.rawEchoMode = 0;
    shell.keyNameDebugMode = 0;
}

/* ************************************************************************** */

// setup the whole shell subsystem
void shell_init(void) {
    // --------------------------------------------------
    // initialize shell
    reset_current_line();
    for (uint8_t i = 0; i < SHELL_HISTORY_LENGTH; i++) {
        reset_history_line(i);
    }

    reset_shell_flags();

    // --------------------------------------------------
    // println(SHELL_VERSION_STRING);
    println("");
    print(SHELL_PROMPT_STRING);
}

/* -------------------------------------------------------------------------- */

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

/* -------------------------------------------------------------------------- */

void shell_update(void) {
    char currentChar = getch();

    // return early if we haven't rx'd a character
    if (currentChar == NULL)
        return;

    if (shell.rawEchoMode) {
        // Echo the input back as a string
        printf("%d ", (int)currentChar);

        // still need to process escape sequences to exit rawEchoMode
        if (currentChar == KEY_ESC) {
            shell.escapeMode = 1;
            return;
        }

        if (shell.escapeMode) {
            process_escape_sequence(currentChar);
            return;
        }

        // return here because we don't process text in rawEchoMode
        println("");
        return;
    }

    // are we currently in an escape sequence?
    if (shell.escapeMode) {
        process_escape_sequence(currentChar);
        return;
    }

    // process control characters
    if (iscntrl(currentChar)) {
        switch (currentChar) {
        case KEY_ESC:
            process_escape_sequence(currentChar);
            return;

        case KEY_CTRL_C:
            print_key_name("ctrl + c");
            return;

        case KEY_CTRL_D: // delete one character to the right of the cursor
            print_key_name("ctrl + d");
            if (shell.cursor != shell.length) {
                remove_char_at_cursor();
            }
            return;

        case KEY_CTRL_E: // move cursor to the end of the line
            print_key_name("ctrl + e");
            move_cursor_to(shell.length);
            return;

        case KEY_CTRL_K: // delete all characters to the right of the cursor
            print_key_name("ctrl + k");
            while (shell.cursor < shell.length) {
                remove_char_at_cursor();
            }
            return;

        case KEY_CTRL_U: // delete all characters to the left of the cursor
            print_key_name("ctrl + u");
            while (shell.cursor > 0) {
                move_cursor(-1);
                remove_char_at_cursor();
            }
            return;

        case KEY_HT:
            print_key_name("tab");
            return;

        case KEY_BS: // delete one character to the left of the cursor
            print_key_name("backspace");
            if (shell.cursor != 0) {
                move_cursor(-1);
                remove_char_at_cursor();
            }
            return;

        case KEY_CR:
            print_key_name("Enter");
            shell.buffer[shell.length] = '\0';
            println("");
            if (shell.length > 0) {
                process_shell_command();

                reset_current_line();
            }
            print(SHELL_PROMPT_STRING);
            return;
        }
    }

    // process printable characters
    if (isprint(currentChar)) {
        insert_char_at_cursor(currentChar);
        return;
    }
}

/* --------------------------------------------------------------------------
 */

void shell_print_commands(void) {
    unsigned char i;

    for (i = 0; i < MAXIMUM_NUM_OF_SHELL_COMMANDS; i++) {
        if (commands.list[i].callback != 0 || commands.list[i].command != 0) {
            println(commands.list[i].command);
        }
    }
}
