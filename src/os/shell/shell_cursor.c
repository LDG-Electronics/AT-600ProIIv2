#include "../../includes.h"
#include "shell_internals.h"

/* ************************************************************************** */

// Cursor movement
void move_cursor(int8_t distance) {
    int8_t newCursor = shell.cursor + distance;

    // don't move left past beginning of line
    if (newCursor < 0) {
        newCursor = 0;
    }

    // don't move right past end of line
    if (newCursor > shell.length) {
        newCursor = shell.length;
    }

    move_cursor_to(newCursor);
}

void move_cursor_to(uint8_t position) {
    // make sure we don't move past the end of the line
    if (position > shell.length) {
        position = shell.length;
    }

    // put cursor at the beginning of the line
    print("\033[64D");

    // move two spaces right to account for the prompt
    print("\033[2C");

    // move the cursor right to the correct spot
    if (position != 0) {
        printf("\033[%dC", position);
    }
    shell.cursor = position;
}

/* -------------------------------------------------------------------------- */

#define save_cursor_location() print("\0337")
#define restore_cursor_location() print("\0338")

void redraw_current_line(void) {
    // put cursor at the beginning of the line
    print("\033[64D");

    // clear line left of cursor
    print("\033[0K");

    // reprint the prompt
    print(SHELL_PROMPT_STRING);

    // reprint existing line
    print(shell.buffer);

    // restore the cursor's original position
    move_cursor_to(shell.cursor);
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

    redraw_current_line();
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

    redraw_current_line();
}