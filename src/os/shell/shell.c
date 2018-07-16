#include "../../includes.h"
#include "shell_internals.h"

/* ************************************************************************** */

line_t shell;

/* ************************************************************************** */

// set up the entire shell subsystem
void shell_init(void) {
    // initialize shell
    memset(&shell, NULL, sizeof(line_t));

    // sequence processing
    shell_sequences_init();

    // shell commands
    shell_commands_init();

    // shell history
    shell_history_init();
}

/* ************************************************************************** */

void process_escape_sequence(key_t key) {
    switch (key.key) {
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

            memset(&shell, NULL, sizeof(line_t));
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
    if (currentChar == NULL) {
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