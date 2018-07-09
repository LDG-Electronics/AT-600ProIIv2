#include "../includes.h"
#include "shell_internals.h"

/* ************************************************************************** */

// line_t history[SHELL_HISTORY_LENGTH];

/* ************************************************************************** */

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