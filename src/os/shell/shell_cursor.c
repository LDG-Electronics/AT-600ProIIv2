#include "../../includes.h"
#include "shell_internals.h"

/* ************************************************************************** */

// Cursor movement
// TODO: refix this bug
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

#define save_cursor_location() print("\0337")
#define restore_cursor_location() print("\0338")

void redraw_current_line(void) {
    // stash current cursor location
    uint8_t cursor = shell.cursor;
    save_cursor_location();

    // put cursor at the beginning of the line
    move_cursor_to(0);

    // clear line left of cursor
    print("\033[0K");

    // backspace two more spaces and reprint the prompt
    printf("\033[%dD", 2);
    print(SHELL_PROMPT_STRING);

    // reprint existing line
    print(shell.buffer);

    // restore the cursor's original position
    restore_cursor_location();
    shell.cursor = cursor;
}

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

    shell.length++;

    // make space for the new char
    uint8_t i = shell.length;
    while (i >= shell.cursor) {
        shell.buffer[i + 1] = shell.buffer[i];
        i--;
    }

    // add the new char
    shell.buffer[shell.cursor] = currentChar;

    save_cursor_location();

    // reprint the rest of the line
    i = shell.cursor;
    while (i < shell.length) {
        putchar(shell.buffer[i]);
        i++;
    }

    restore_cursor_location();

    move_cursor(1);
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

    save_cursor_location();

    // reprint the rest of the line
    i = shell.cursor;
    while (shell.buffer[i] != NULL) {
        putchar(shell.buffer[i]);
        i++;
    }

    restore_cursor_location();
}