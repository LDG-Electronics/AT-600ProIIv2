#include "../../includes.h"
#include "shell_internals.h"

/* ************************************************************************** */

shell_t shell;

/* ************************************************************************** */

// reset shell.buffer to 0
void reset_current_line(void) {
    memset(&shell.buffer, NULL, sizeof(line_t));

    shell.length = 0;
    shell.cursor = 0;
}

// reset all shell state flags to default values
void reset_shell_flags(void) {
    shell.escapeMode = 0;
    shell.sequenceInspectionMode = 0;
}

/* ************************************************************************** */

// setup the whole shell subsystem
void shell_init(void) {
    // --------------------------------------------------
    // initialize shell
    reset_current_line();
    reset_shell_flags();

    // --------------------------------------------------
    // shell commands
    shell_commands_init();

    // --------------------------------------------------
    // shell history
    shell_history_init();

    // --------------------------------------------------
    // println(SHELL_VERSION_STRING);
    println("");
    print(SHELL_PROMPT_STRING);
}

/* -------------------------------------------------------------------------- */

void shell_update(void) {
    char currentChar = getch();

    // return early if we haven't rx'd a character
    if (currentChar == NULL)
        return;

    if (shell.sequenceInspectionMode) {
        // Echo the input back as a string
        printf("%d ", (int)currentChar);
    }

    // Deal with escape sequences
    if (iscntrl(currentChar) || currentChar == KEY_ESC || shell.escapeMode) {
        process_escape_sequence(currentChar);
        return;
    }

    if (shell.sequenceInspectionMode) {
        // return here because we don't process text in sequenceInspectionMode
        println("");
        return;
    }

    // process printable characters
    if (isprint(currentChar)) {
        insert_char_at_cursor(currentChar);
        return;
    }
}