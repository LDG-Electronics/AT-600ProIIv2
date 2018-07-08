#include "../includes.h"
#include "shell_keycodes.h"

/* ************************************************************************** */
/*  shell command list

    This data structure is the central registry for shell commands.

    Individual commands should be defined in shell_commands.c and registered in
    shell_commands_init().

    The function signature of a shell command must be:
    int (*shell_program_t) (int, char **)

    shell_command_t contains a function pointer to the shell command body, plus
    a pointer to string that represents the command that needs to be typed.

    command_list_t contains an array of shell_command_t objects, and
    numOfRegisteredCommands, which
*/
typedef struct {
    shell_program_t callback;
    const char *command;
} shell_command_t;

typedef struct commands {
    shell_command_t list[MAXIMUM_NUM_OF_SHELL_COMMANDS];
    uint8_t numOfRegisteredCommands;
} command_list_t;

static command_list_t commands;

void init_shell_commands(void) {
    for (uint8_t i = 0; i < MAXIMUM_NUM_OF_SHELL_COMMANDS; i++) {
        commands.list[i].callback = NULL;
        commands.list[i].command = NULL;
    }
    commands.numOfRegisteredCommands = 0;
}

/* -------------------------------------------------------------------------- */
#define ESCAPE_BUFFER_LENGTH 10

typedef struct {
    char buffer[SHELL_BUFFER_LENGTH];
    uint8_t length;
    uint8_t cursorLocation;
    unsigned escapeMode : 1;
    unsigned rawEchoMode : 1;
} shell_t;

shell_t shell;

void reset_shell_buffer(void) {
    for (uint8_t i = 0; i < SHELL_BUFFER_LENGTH; i++) {
        shell.buffer[i] = 0;
    }
    shell.length = 0;
    shell.cursorLocation = 0;
}

void init_shell_data(void) {
    reset_shell_buffer();
    shell.escapeMode = 0;
    shell.rawEchoMode = 0;
}

/* ************************************************************************** */

void shell_init(void) {
    init_shell_commands();
    init_shell_data();

    // println(SHELL_VERSION_STRING);
    println("");
    print(SHELL_PROMPT_STRING);
}

bool shell_register(shell_program_t program, const char *string) {
    unsigned char i;

    for (i = 0; i < MAXIMUM_NUM_OF_SHELL_COMMANDS; i++) {
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

void move_cursor(int8_t distance) {
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

void move_cursor_to(uint8_t position) {
    // cursor is already where it needs to end up
    if (shell.cursorLocation == position) {
        return;
    }

    if (position > shell.length) {
        position = shell.length;
    }

    // 0 is the home position
    if (position == 0) {
        while (shell.cursorLocation > 0) {
            move_cursor(-1);
        }
    }

    // need to move right
    while (shell.cursorLocation < position) {
        move_cursor(1);
    }

    // need to move right
    while (shell.cursorLocation > position) {
        move_cursor(-1);
    }
}

/* -------------------------------------------------------------------------- */

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

    // make space for the new char
    uint8_t i = shell.length;
    while (i > shell.cursorLocation) {
        shell.buffer[i + 1] = shell.buffer[i];
        i--;
    }
    shell.length++;

    // add the new char
    shell.buffer[shell.cursorLocation] = currentChar;

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

typedef struct {
    char buffer[ESCAPE_BUFFER_LENGTH];
    uint8_t length;
} escape_t;

escape_t escape;

void reset_escape_buffer(void) {
    for (uint8_t i = 0; i < ESCAPE_BUFFER_LENGTH; i++) {
        escape.buffer[i] = 0;
    }
    escape.length = 1;
}

/*  Notes on escape sequences:

    If a block returns, that means that the escape sequence isn't over yet.

    If a block ends with "goto FINISHED;", that means that we know it was a
    valid escape sequence.
*/
void process_escape_sequence(char currentChar) {
    if (shell.escapeMode == 0) {
        shell.escapeMode = 1;
        reset_escape_buffer();
        return;
    }

    char prevChar = escape.buffer[escape.length];
    escape.length++;
    escape.buffer[escape.length] = currentChar;

    switch (escape.length) {
    case 0:
    case 1:
    case 2:
        return;
    case 3:
        switch (currentChar) {
        case KEY_UP:
            // print("up");
            goto FINISHED;
        case KEY_DOWN:
            // print("down");
            goto FINISHED;
        case KEY_RIGHT:
            // print("right");
            move_cursor(1);
            goto FINISHED;
        case KEY_LEFT:
            // print("left");
            move_cursor(-1);
            goto FINISHED;
        case KEY_HOME:
            // print("home");
            move_cursor_to(0);
            goto FINISHED;
        case KEY_END:
            // print("end");
            move_cursor_to(shell.length);
            goto FINISHED;
        case KEY_F1:
            // print("F1");
            goto FINISHED;
        case KEY_F2:
            // print("F2");
            goto FINISHED;
        case KEY_F3:
            // print("F3");
            goto FINISHED;
        case KEY_F4:
            // print("F4");
            toggle_raw_echo_mode();
            goto FINISHED;
        }
        return;
    case 4:
        switch (currentChar) {
        case '~':
            switch (prevChar) {
            case KEY_PGUP:
                // print("pgup");
                goto FINISHED;
            case KEY_PGDN:
                // print("pgdn");
                goto FINISHED;
            case KEY_DEL:
                // println("del");
                if (shell.cursorLocation != shell.length) {
                    remove_char_at_cursor();
                }
                goto FINISHED;
            case KEY_INS:
                // print("ins");
                goto FINISHED;
            }
        }
        return;
    case 5:
        if (currentChar == '~') {
            switch (prevChar) {
            case KEY_F5:
                print("F5");
                goto FINISHED;
            case KEY_F6:
                print("F6");
                goto FINISHED;
            case KEY_F7:
                print("F7");
                goto FINISHED;
            case KEY_F8:
                print("F8");
                goto FINISHED;
            case KEY_F9:
                print("F9");
                goto FINISHED;
            case KEY_F10:
                print("F10");
                goto FINISHED;
            case KEY_F11:
                print("F11");
                goto FINISHED;
            case KEY_F12:
                print("F12");
                goto FINISHED;
            }
        }
        return;
    case 6:
        switch (currentChar) {
        case KEY_RIGHT:
            // print("^right");
            goto FINISHED;
        case KEY_LEFT:
            // print("^left");
            goto FINISHED;
        case KEY_HOME:
            // print("^home");
            goto FINISHED;
        case KEY_END:
            // print("^end");
            goto FINISHED;
        case '~':
            switch (escape.buffer[3]) {
            case KEY_HOME:
                // print("^delete");
                goto FINISHED;
            case KEY_END:
                // print("^insert");
                goto FINISHED;
            }
        }
        return;
    case 7:
        goto FINISHED;

    FINISHED:
        // if we're in raw echo mode, then add a newline to seperate groups
        // of escape sequence codes
        if (shell.rawEchoMode) {
            printf(" length: %d\r\n", escape.length);
            println("");
        }

        // If we reach this spot, then we've fully processed the current
        // escape sequence, and we can exit escape mode.
        reset_escape_buffer();
        shell.escapeMode = 0;
    }
}

void shell_update(void) {
    char currentChar = getch();

    // return early if we haven't rx'd a character
    if (currentChar == KEY_NUL)
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

        case KEY_ETX:
            // println("^c");
            return;

        case KEY_HT:
            // println("tab");
            return;

        case KEY_BS:
            if (shell.cursorLocation != 0) {
                move_cursor(-1);
                remove_char_at_cursor();
            }
            return;

        case KEY_CR:
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
