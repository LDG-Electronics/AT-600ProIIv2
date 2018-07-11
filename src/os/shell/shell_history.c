#include "../../includes.h"
#include "shell_internals.h"

/* ************************************************************************** */

// this is disgusting. Thank you pic18 paged memory...
#define HISTORY_LIST                                                           \
    X(0)                                                                       \
    X(1)                                                                       \
    X(2)                                                                       \
    X(3)                                                                       \
    X(4)                                                                       \
    X(5)                                                                       \
    X(6)                                                                       \
    X(7)                                                                       \
    X(8)                                                                       \
    X(9)                                                                       \
    X(10)                                                                      \
    X(11)                                                                      \
    X(12)                                                                      \
    X(13)                                                                      \
    X(14)                                                                      \
    X(15)

#define X(NAME) line_t history_##NAME;
HISTORY_LIST
#undef X

/* -------------------------------------------------------------------------- */

typedef struct {
    line_t *line[SHELL_HISTORY_LENGTH];
    line_t tempLine;
    uint8_t head;
    uint8_t pointer;
    uint8_t length;
} shell_history_t;

shell_history_t history;

// clear a line in the shell history
static void clear_history_line(uint8_t line) {
    memset(history.line[line], NULL, sizeof(line_t));
}

void shell_history_wipe(void) {
    for (uint8_t i = 0; i < SHELL_HISTORY_LENGTH; i++) {
        clear_history_line(i);
    }

    memset(&history.tempLine, NULL, sizeof(line_t));

    history.head = 0;
    history.pointer = 0;
    history.length = 0;
}

/* ************************************************************************** */

void shell_history_init(void) {
    // initialize history array with pointers to line_t's
#define X(NAME) history.line[NAME] = &history_##NAME;
    HISTORY_LIST
#undef X

    shell_history_wipe();
}

/* -------------------------------------------------------------------------- */
/*

    history.pointer is higher the further into the past we got

    history.pointer is used as an offset from history.head
*/

// push the current line onto the history buffer
// used when we hit the enter key
void shell_history_push(void) {
    // Decrement head
    history.head = (history.head - 1) % SHELL_HISTORY_LENGTH;
    if (history.length < SHELL_HISTORY_LENGTH) {
        history.length++;
    }

    history.pointer = 0;

    memcpy(history.line[history.head], &shell.buffer, sizeof(line_t));
}

// used when we hit the up arrow
void shell_history_show_older(void) {
    // if history is empty then what are we doing?
    if (history.length == 0) {
        return;
    }

    // if the pointer is at 0, the stash the original line
    if(history.pointer == 0) {
        memcpy(&shell.buffer, &history.tempLine, sizeof(line_t));
    }

    if (history.pointer < SHELL_HISTORY_LENGTH - 1 &&
        history.pointer < history.length) {
        history.pointer++;
    }

    uint8_t line = (history.head + history.pointer - 1) % SHELL_HISTORY_LENGTH;

    memcpy(&shell.buffer, history.line[line], sizeof(line_t));
    redraw_current_line();
}

// used when we hit the down arrow
void shell_history_show_newer(void) {
    // if history is empty then what are we doing?
    if (history.length == 0) {
        return;
    }

    // if the pointer is already at 0, then redraw the original line
    if (history.pointer == 0) {
        memcpy(&shell.buffer, &history.tempLine, sizeof(line_t));
        redraw_current_line();
        return;
    }

    if (history.pointer != 0) {
        history.pointer--;
    }

    uint8_t line = (history.head + history.pointer - 1) % SHELL_HISTORY_LENGTH;

    memcpy(&shell.buffer, history.line[line], sizeof(line_t));
    redraw_current_line();
}