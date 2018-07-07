#include "../includes.h"
#include "shell_keycodes.h"

/* ************************************************************************** */

typedef struct {
    shell_program_t callback;
    const char *command;
} shell_entry_t;

typedef struct commands {
    shell_entry_t list[CONFIG_SHELL_MAX_COMMANDS];
    uint8_t numOfRegisteredCommands;
} command_list_t;

command_list_t commands;

void init_shell_commands(void) {
    for (uint8_t i = 0; i < CONFIG_SHELL_MAX_COMMANDS; i++) {
        commands.list[i].callback = NULL;
        commands.list[i].command = NULL;
    }
    commands.numOfRegisteredCommands = 0;
}

/* -------------------------------------------------------------------------- */

typedef struct {
    char buffer[SHELL_BUFFER_LENGTH];
    char prevChar;
    uint8_t length;
    uint8_t cursorLocation;
    unsigned escapeMode : 1;
    unsigned rawEchoMode : 1;
} shell_buffer_t;

shell_buffer_t shell;

void init_shell_data(void) {
    for (uint8_t i = 0; i < SHELL_BUFFER_LENGTH; i++) {
        shell.buffer[i] = 0;
    }
    shell.length = 0;
    shell.cursorLocation = 0;
    shell.escapeMode = 0;
    shell.rawEchoMode = 0;
}

void reset_shell_buffer(void) {
    for (uint8_t i = 0; i < SHELL_BUFFER_LENGTH; i++) {
        shell.buffer[i] = 0;
    }
    shell.length = 0;
    shell.cursorLocation = 0;
}

/* ************************************************************************** */

void shell_init(void) {
    init_shell_commands();
    init_shell_data();

    println(SHELL_VERSION_STRING);
    print(SHELL_PROMPT_STRING);
}

bool shell_register(shell_program_t program, const char *string) {
    unsigned char i;

    for (i = 0; i < CONFIG_SHELL_MAX_COMMANDS; i++) {
        if (commands.list[i].callback != 0 || commands.list[i].command != 0)
            continue;
        commands.list[i].callback = program;
        commands.list[i].command = string;
        return true;
    }
    return false;
}

/* -------------------------------------------------------------------------- */

int8_t find_matching_command(char *string) {
    for (uint8_t i = 0; i < CONFIG_SHELL_MAX_COMMANDS; i++) {
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
    char toggle = 0;

    char *argv_list[CONFIG_SHELL_MAX_COMMAND_ARGS];

    argv_list[argc] = &shell.buffer[0];

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
// Cursor movement

void move_cursor(int16_t distance) {
    // move right
    if (distance > 0) {
        if (shell.cursorLocation < shell.length) {
            shell.cursorLocation++;
            printf("\033[%dC", distance);
        }
    }

    // move left
    if (distance < 0) {
        if (shell.cursorLocation > 0) {
            shell.cursorLocation--;
            printf("\033[%dD", -(distance));
        }
    }
}

void move_cursor_home(void) {
    while (shell.cursorLocation > 0) {
        move_cursor(-1);
    }
}

void move_cursor_end(void) {
    while (shell.cursorLocation < shell.length) {
        move_cursor(1);
    }
}

void insert_char_at_cursor(char currentChar) {
    if (shell.length >= CONFIG_SHELL_MAX_INPUT) {
        return;
    }

    // process is easier if cursor is already at end of line
    if (shell.cursorLocation == shell.length) {
        // add the new char
        putch(currentChar);
        shell.buffer[shell.cursorLocation] = currentChar;

        shell.length++;
        shell.cursorLocation++;

        // and we're done
        return;
    }

    // TODO: insertion only half works

    // make space for the new char
    uint8_t i = shell.length;
    while (i > shell.cursorLocation) {
        shell.buffer[i + 1] = shell.buffer[i];
        i--;
    }
    shell.length++;

    // add the new char
    shell.buffer[shell.cursorLocation] = currentChar;

    // clear from cursor to end of line
    print("\033[0K");

    // save cursor location
    print("\0337");

    // reprint the rest of the line
    i = shell.cursorLocation;
    while (i < shell.length) {
        putchar(shell.buffer[i]);
        i++;
    }

    // restore cursor location
    print("\0338");

    shell.cursorLocation++;
}

void remove_char_at_cursor(void) {
    if (shell.length == 0) {
        return;
    }

    if (shell.cursorLocation == shell.length) {
        return;
    }

    uint8_t i = shell.cursorLocation;

    while (shell.buffer[i] != NULL) {
        shell.buffer[i] = shell.buffer[i + 1];
        i++;
    }
    shell.buffer[i - 1] = NULL;
    shell.buffer[i] = NULL;
    shell.buffer[i + 1] = NULL;
    shell.length--;

    // clear from cursor to end of line
    print("\033[0K");

    // save cursor location
    print("\0337");

    // reprint the rest of the line
    i = shell.cursorLocation;
    while (shell.buffer[i] != NULL) {
        putchar(shell.buffer[i]);
        i++;
    }

    // restore cursor location
    print("\0338");
}

/* -------------------------------------------------------------------------- */

void toggle_raw_echo_mode(void) {
    shell.rawEchoMode = !shell.rawEchoMode;
    if (shell.rawEchoMode == 1) {
        println("Entering Raw Echo Mode.");
    } else {
        println("Leaving Raw Echo Mode.");
        print(SHELL_PROMPT_STRING);
    }
}

/* -------------------------------------------------------------------------- */
/*  Notes on escape sequences:

    If a block returns, that means that the escape sequence isn't over yet.

    If a block breaks, that means it's the final byte of an escape sequence, and
    it's going to fall through to the bottom of the function and exit escape
    mode.
*/
void process_escape_sequence(char currentChar) {
    if ((currentChar == '[') || (currentChar == KEY_FN)) {
        shell.prevChar = currentChar;
        return;
    }

    if (currentChar == '~') {
        switch (shell.prevChar) {
        case KEY_DEL:
            // println("del");
            if (shell.cursorLocation != shell.length) {
                remove_char_at_cursor();
            }
            break;
        case KEY_INS:
            // println("ins");
            break;
        case KEY_PGUP:
            // println("pgup");
            break;
        case KEY_PGDN:
            // println("pgdn");
            break;
        }
    }

    if (shell.prevChar == '[') {
        switch (currentChar) {
        case KEY_DEL:
        case KEY_INS:
        case KEY_PGUP:
        case KEY_PGDN:
            shell.prevChar = currentChar;
            return;
        case KEY_UP:
            // println("up");
            break;
        case KEY_DOWN:
            // println("down");
            break;
        case KEY_RIGHT:
            // println("right");
            move_cursor(1);
            break;
        case KEY_LEFT:
            // println("left");
            move_cursor(-1);
            break;
        case KEY_HOME:
            // println("home");
            // move_cursor(-(shell.cursorLocation));
            move_cursor_home();
            break;
        case KEY_END:
            // println("end");
            // move_cursor(shell.length - shell.cursorLocation);
            move_cursor_end();
            break;
        }
    }

    if (shell.prevChar == KEY_FN) {
        switch (currentChar) {
        case KEY_F1:
            // println("F1");
            break;
        case KEY_F2:
            // println("F2");
            break;
        case KEY_F3:
            // println("F3");
            break;
        case KEY_F4:
            // println("F4");
            toggle_raw_echo_mode();
            break;
        }
    }

    // if we're in raw echo mode, then add a newline to seperate groups of
    // escape sequence codes
    if (shell.rawEchoMode) {
        println("");
    }

    // If we reach this spot, then we've fully processed the current escape
    // sequence, and we can exit escape mode.
    shell.escapeMode = 0;
}

void shell_update(void) {
    char currentChar = getch();

    // return early if we haven't rx'd a character
    if (currentChar == KEY_NUL)
        return;

    if (shell.rawEchoMode) {
        // Echo the input back as a string
        printf("%d\r\n", (int)currentChar);

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
        if (currentChar == KEY_ESC) {
            shell.escapeMode = 1;
            return;
        }

        if (currentChar == KEY_ETX) {
            // println("ctrl+c");
            return;
        }

        if (currentChar == KEY_HT) {
            // println("tab");
            return;
        }

        if (currentChar == KEY_BS) {
            if (shell.cursorLocation != 0) {
                move_cursor(-1);
                remove_char_at_cursor();
            }
            return;
        }

        if (currentChar == KEY_CR) {
            shell.buffer[shell.length] = '\0';
            println("");
            if (shell.length > 0) {
                process_shell_command();

                reset_shell_buffer();
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

/* -------------------------------------------------------------------------- */

void shell_print_commands(void) {
    unsigned char i;

    for (i = 0; i < CONFIG_SHELL_MAX_COMMANDS; i++) {
        if (commands.list[i].callback != 0 || commands.list[i].command != 0) {
            println(commands.list[i].command);
        }
    }
}
