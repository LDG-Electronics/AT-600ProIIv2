#include "../includes.h"
#include "shell_keycodes.h"

/* ************************************************************************** */

/*  line_t

    A shell line contains a char buffer that is SHELL_MAX_LENGTH long, and two
    variables to keep track of the current length of the buffer and the current
    location of the cursor.
*/
typedef struct {
    char buffer[SHELL_MAX_LENGTH];
    uint8_t length;
    uint8_t cursor;
} line_t;

// alternate line_t definition with an anonymous array
typedef struct {
    char[SHELL_MAX_LENGTH];
    uint8_t length;
    uint8_t cursor;
} alt_line_t;

/* -------------------------------------------------------------------------- */

/*  shell_flags_t stores flags to keep track of various shell modes

    escapeMope
    This mode is used to process escape sequences. escapeMode is entered the
    shell receives an escape character(KEY_ESC). Sequences commonly contains
    printable ascii characters, so we make sure not to process printable
    characters while in escapeMode. escapeMode is exited after an escape
    sequence is successfully processed.

    rawEchoMode
    This is a debug mode used to diagnose escape sequences
*/
typedef union {
    struct {
        unsigned escapeMode : 1;
        unsigned rawEchoMode : 1;
    };
    uint8_t allFlags;
} shell_flags_t;

/* -------------------------------------------------------------------------- */

/*  shell_t current state of the shell

*/
typedef struct {
    line_t; // <- NOT PORTABLE - anonymous member struct via typedef
    line_t history[SHELL_HISTORY_LENGTH];
    shell_flags_t; // <- NOT PORTABLE - anonymous member struct via typedef
} shell_t;

shell_t shell;
// line_t history[SHELL_HISTORY_LENGTH];

/* ************************************************************************** */

// reset shell.buffer to 0
void reset_current_line(void) {
    memset(&shell.buffer[0], NULL, SHELL_MAX_LENGTH);

    shell.length = 0;
    shell.cursor = 0;
}

// reset a line in the shell history
void reset_history_line(uint8_t line) {
    memset(&shell.history[line].buffer[0], NULL, SHELL_MAX_LENGTH);

    shell.history[line].length = 0;
    shell.history[line].cursor = 0;
}

// Store the current line in the history buffer
void copy_current_line_to_history(uint8_t line) {
    memcpy(&shell.history[line].buffer[0], &shell.buffer[0], sizeof(line_t));

    // shell.history[line].buffer[i] = shell.buffer[i];
    shell.history[line].length = shell.length;
    shell.history[line].cursor = shell.cursor;
}

// Copy a line from the history buffer to the current line
void copy_current_line_from_history(uint8_t line) {
    memcpy(&shell.buffer[0], &shell.history[line].buffer[0], sizeof(line_t));

    // shell.buffer[i] = shell.history[line].buffer[i];
    shell.length = shell.history[line].length;
    shell.cursor = shell.history[line].cursor;
}

// reset all shell state flags to default values
void reset_shell_flags(void) {
    shell.escapeMode = 0;
    shell.rawEchoMode = 0;
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
        if (shell.cursor < shell.length) {
            shell.cursor++;
            printf("\033[%dC", distance);
        }
    }

    // move left
    if (distance < 0) {
        if (shell.cursor > 0) {
            shell.cursor--;
            printf("\033[%dD", -(distance));
        }
    }
}

void move_cursor_to(uint8_t position) {
    // cursor is already where it needs to end up
    if (shell.cursor == position) {
        return;
    }

    // make sure we don't move past the end of the line
    if (position > shell.length) {
        position = shell.length;
    }

    // need to move right
    while (shell.cursor < position) {
        move_cursor(1);
    }

    // need to move right
    while (shell.cursor > position) {
        move_cursor(-1);
    }
}

/* -------------------------------------------------------------------------- */

void insert_char_at_cursor(char currentChar) {
    if (shell.length >= SHELL_MAX_LENGTH) {
        return;
    }

    // process is easier if cursor is already at end of line
    if (shell.cursor == shell.length) {
        // add the new char
        putch(currentChar);
        shell.buffer[shell.cursor] = currentChar;

        shell.length++;
        shell.cursor++;

        // and we're done
        return;
    }

    // make space for the new char
    uint8_t i = shell.length;
    while (i > shell.cursor) {
        shell.buffer[i + 1] = shell.buffer[i];
        i--;
    }
    shell.length++;

    // add the new char
    shell.buffer[shell.cursor] = currentChar;

    // save cursor location
    print("\0337");

    // reprint the rest of the line
    i = shell.cursor;
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

    if (shell.cursor == shell.length) {
        return;
    }

    uint8_t i = shell.cursor;

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
    i = shell.cursor;
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

#define ESCAPE_BUFFER_LENGTH 10

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
        // fallthrough
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
                if (shell.cursor != shell.length) {
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
                // print("F5");
                goto FINISHED;
            case KEY_F6:
                // print("F6");
                goto FINISHED;
            case KEY_F7:
                // print("F7");
                goto FINISHED;
            case KEY_F8:
                // print("F8");
                goto FINISHED;
            case KEY_F9:
                // print("F9");
                toggle_raw_echo_mode();
                goto FINISHED;
            case KEY_F10:
                // print("F10");
                goto FINISHED;
            case KEY_F11:
                // print("F11");
                goto FINISHED;
            case KEY_F12:
                // print("F12");
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
            case KEY_DEL:
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
            if (shell.cursor != 0) {
                move_cursor(-1);
                remove_char_at_cursor();
            }
            return;

        case KEY_CR:
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
