#include "shell.h"
#include "../serial_port.h"
#include "shell_command_processor.h"
#include "shell_cursor.h"
#include "shell_history.h"
#include "shell_keys.h"
#include <ctype.h>
#include <string.h>

/* ************************************************************************** */

// stores linebuffer and cursor position
shell_line_t shell;

/* -------------------------------------------------------------------------- */

shell_program_t shellCallback;

void shell_set_program_callback(shell_program_t callback) {
    shellCallback = callback;
}

/* ************************************************************************** */

// set up the entire shell subsystem
void shell_init(void) {
    serial_port_init();

    // initialize shell
    memset(&shell, 0, sizeof(shell_line_t));
    shell_set_program_callback(NULL);

    // shell history
    shell_history_init();
}

/* ************************************************************************** */

void process_escape_sequence(key_t key) {
    switch (key.key) {
    default:
        return;
    case BACKSPACE: // delete one character to the left of the cursor
        if (shell.cursor != 0) {
            move_cursor(-1);
            remove_char_at_cursor();
        }
        return;
    case ENTER:
        println("");
        if (shell.length > 0) {
            shell.buffer[shell.length] = '\0';
            // Push the current line to history before it gets mangled
            shell_history_push();
            process_shell_command();

            memset(&shell, 0, sizeof(shell_line_t));
            return;
        }
        print(SHELL_PROMPT_STRING);
        return;
    case UP:
        shell_history_show_older();
        return;
    case DOWN:
        shell_history_show_newer();
        return;
    case LEFT:
        switch (key.mod) {
        default:
            move_cursor(-1);
            return;
        case CTRL:
            do {
                move_cursor(-1);
            } while (shell.buffer[shell.cursor] != ' ' && shell.cursor > 0);
            return;
        }
    case RIGHT:
        switch (key.mod) {
        default:
            move_cursor(1);
            return;
        case CTRL:
            do {
                move_cursor(1);
            } while (shell.buffer[shell.cursor] != ' ' &&
                     shell.cursor < shell.length);
            return;
        }
    case HOME:
        move_cursor_to(0);
        return;
    case END:
        move_cursor_to(shell.length);
        return;
    case DELETE:
        remove_char_at_cursor();
        return;
    case F7:
        toggle_history_inspection_mode();
        return;
    }
}

void shell_update(void) {
    char currentChar = getch();

    // return early if we haven't rx'd a character
    if (currentChar == 0) {
        return;
    }

    // execute shell callback, if one is registered
    if (shellCallback) {
        // ctrl+c will terminate a running shell program
        if (currentChar == 3) {
            shell_set_program_callback(NULL);

            print("\033[2J");
            println("");
            print(SHELL_PROMPT_STRING);
            return;
        }
        char *argv_list[CONFIG_SHELL_MAX_COMMAND_ARGS];
        argv_list[0] = &currentChar;
        shellCallback(1, argv_list);
        return;
    }

    // control characters and escape sequences
    if (iscntrl(currentChar)) {
        key_t key = identify_key(currentChar);
        process_escape_sequence(key);
        return;
    }

    // finally, printable characters
    if (isprint(currentChar)) {
        insert_char_at_cursor(currentChar);
        return;
    }
}