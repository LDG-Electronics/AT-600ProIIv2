#include "shell.h"
#include "../serial_port.h"
#include "shell_command_processor.h"
#include "shell_cursor.h"
#include "shell_history.h"
#include "shell_keys.h"
#include "shell_utils.h"
#include <ctype.h>
#include <string.h>

/* ************************************************************************** */

// stores linebuffer and cursor position
shell_line_t shell;

/* -------------------------------------------------------------------------- */

shell_callback_t shellCallback;

void shell_register_callback(shell_callback_t callback) {
    shellCallback = callback;
}

static void terminate_current_program(void) {
    shell_register_callback(NULL);

    term_reset_screen();
    println("");
    print(SHELL_PROMPT_STRING);
}
/* ************************************************************************** */

// set up the entire shell subsystem
void shell_init(pps_input_t rxPin, pps_output_t *txPin) {
    serial_port_init(rxPin, txPin);
    
    // initialize shell
    memset(&shell, 0, sizeof(shell_line_t));
    shell_register_callback(NULL);

    // shell history
    shell_history_init();
}

/* ************************************************************************** */

void process_escape_sequence(key_t key) {
    switch (key.key) {
    default: // unrecognized keys don't do anything
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
            // add terminating null to shell buffer
            shell.buffer[shell.length] = '\0';

            // Push the current line to history before it gets mangled
            shell_history_push();

            // attempt to process the shell buffer
            if (process_shell_command(&shell.buffer[0]) == -1) {
                // if there's no valid command, say something
                printf("%s: command not found\r\n", shell.buffer);
            }

            // wipe the current shell line
            memset(&shell, 0, sizeof(shell_line_t));

            // only print a new prompt if no callback is registered
            if (!shellCallback) {
                print(SHELL_PROMPT_STRING);
            }
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

/* -------------------------------------------------------------------------- */

void shell_update(void) {
    char currentChar = getch();

    // return early if we haven't rx'd a character
    if (currentChar == 0) {
        return;
    }

    // execute shell callback, if one is registered
    if (shellCallback) {
        // ctrl+c forces the program to terminate
        if (currentChar == 3) {
            terminate_current_program();
            return;
        }
        // execute callback
        if (shellCallback(currentChar) == -1) {
            terminate_current_program();
        }
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