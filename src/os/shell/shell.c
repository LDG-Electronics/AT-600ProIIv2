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

/* -------------------------------------------------------------------------- */

enum {
    KEY_CTRL_C = 3,
    KEY_CTRL_D = 4,
    KEY_CTRL_E = 5,
    KEY_CTRL_K = 11,
    KEY_CTRL_U = 21,
    KEY_CTRL_Y = 25,
    KEY_CTRL_Z = 26,
    KEY_CTRL_CR = 10,
    KEY_CTRL_BS = 31,
    KEY_BS = 8,
    KEY_HT = 9,
    KEY_LF = 10,
    KEY_CR = 13,
} controlCharacters;

/* ************************************************************************** */

// escape sequence codes
#define KEY_ESC 27

void process_control_character(char currentChar) {
    switch (currentChar) {
    case KEY_CTRL_D: // delete one character to the right of the cursor
        remove_char_at_cursor();
        return;
    case KEY_CTRL_E: // move cursor to the end of the line
        move_cursor_to(shell.length);
        return;
    case KEY_CTRL_K: // delete all characters to the right of the cursor
        while (shell.cursor < shell.length) {
            remove_char_at_cursor();
        }
        return;
    case KEY_CTRL_U: // delete all characters to the left of the cursor
        while (shell.cursor > 0) {
            move_cursor(-1);
            remove_char_at_cursor();
        }
        return;
    case KEY_BS: // delete one character to the left of the cursor
        if (shell.cursor != 0) {
            move_cursor(-1);
            remove_char_at_cursor();
        }
        return;
    case KEY_CR:
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
    default:
        return;
    }
}

void process_escape_sequence(key_t key) {
    switch (key.key) {
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

    // control characters have first priority
    if (iscntrl(currentChar)) {
        process_control_character(currentChar);
    }

    // escape sequences are next
    if (currentChar == KEY_ESC) {
        key_t key = intercept_escape_sequence();
        process_escape_sequence(key);
        return;
    }

    // finally, printable characters
    if (isprint(currentChar)) {
        insert_char_at_cursor(currentChar);
        return;
    }
}