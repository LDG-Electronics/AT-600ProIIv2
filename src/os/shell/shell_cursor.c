#include "shell_cursor.h"
#include "../serial_port.h"

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
    print("\033[100D");

    // move two spaces right to account for the prompt
    print("\033[2C");

    // move the cursor right to the correct spot
    if (position != 0) {
        printf("\033[%dC", position);
    }
    shell.cursor = position;
}

/* -------------------------------------------------------------------------- */

void draw_line(shell_line_t *line) {
    // put cursor at the beginning of the line
    print("\033[100D");

    // move two spaces right to account for the prompt
    print("\033[2C");

    // clear line right of cursor
    print("\033[0K");

    // reprint existing line
    print(line->buffer);

    // restore the cursor's original position
    move_cursor_to(line->cursor);
}

void draw_line_from_cursor(shell_line_t *line) {
    // clear line right of cursor
    print("\033[0K");

    // reprint existing line
    print(&line->buffer[line->cursor]);

    // restore the cursor's original position
    move_cursor_to(line->cursor);
}

// TODO: replace magic number with LENGTH_OF_SHELL_PROMPT
void insert_char_at_cursor(char currentChar) {
    if (shell.length >= SHELL_MAX_LENGTH - 2) {
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

    draw_line_from_cursor(&shell);
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

    while (shell.buffer[i] != 0) {
        shell.buffer[i] = shell.buffer[i + 1];
        i++;
    }
    shell.buffer[i - 1] = 0;
    shell.buffer[i] = 0;
    shell.buffer[i + 1] = 0;
    shell.length--;

    draw_line_from_cursor(&shell);
}