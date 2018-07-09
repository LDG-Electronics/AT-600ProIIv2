#include "../../includes.h"
#include "shell_internals.h"

/* ************************************************************************** */

shell_t shell;

/* ************************************************************************** */

// reset shell.buffer to 0
void reset_current_line(void) {
    memset(&shell.buffer[0], NULL, SHELL_MAX_LENGTH);

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
    for (uint8_t i = 0; i < SHELL_HISTORY_LENGTH; i++) {
        reset_history_line(i);
    }

    reset_shell_flags();

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
    if (currentChar == KEY_ESC || shell.escapeMode) {
        process_escape_sequence(currentChar);
        return;
    }

    // process control characters
    if (iscntrl(currentChar)) {
        process_control_character(currentChar);
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

/* -------------------------------------------------------------------------- */

void shell_print_commands(void) {
    unsigned char i;

    for (i = 0; i < MAXIMUM_NUM_OF_SHELL_COMMANDS; i++) {
        if (commands.list[i].callback != 0 || commands.list[i].command != 0) {
            println(commands.list[i].command);
        }
    }
}
