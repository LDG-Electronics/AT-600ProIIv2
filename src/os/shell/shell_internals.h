#ifndef _SHELL_INTERNALS_H_
#define _SHELL_INTERNALS_H_

/* ************************************************************************** */

#include "shell_escapes.h"
#include "shell_history.h"
#include "shell_keycodes.h"
#include "shell_cursor.h"

/* ************************************************************************** */

/*  line_t

    A shell line contains a char buffer that is SHELL_MAX_LENGTH long, and two
    variables to keep track of the current length of the buffer and the current
    location of the cursor.
*/
typedef struct {
    char buffer[SHELL_MAX_LENGTH];
    uint8_t length;
    uint8_t cursor;
} line_t;

// alternate line_t definition with an anonymous array
typedef struct {
    char[SHELL_MAX_LENGTH];
    uint8_t length;
    uint8_t cursor;
} alt_line_t;

/* -------------------------------------------------------------------------- */

/*  shell_flags_t stores flags to keep track of various shell modes

    escapeMope
    This mode is used to process escape sequences. escapeMode is entered the
    shell receives an escape character(KEY_ESC). Sequences commonly contains
    printable ascii characters, so we make sure not to process printable
    characters while in escapeMode. escapeMode is exited after an escape
    sequence is successfully processed.

    rawEchoMode
    This is a debug mode used to diagnose escape sequences
*/
typedef union {
    struct {
        unsigned escapeMode : 1;
        unsigned rawEchoMode : 1;
        unsigned keyNameDebugMode : 1;
    };
    uint8_t allFlags;
} shell_flags_t;

/* -------------------------------------------------------------------------- */

/*  shell_t current state of the shell

*/
typedef struct {
    line_t; // <- NOT PORTABLE - anonymous member struct via typedef
    line_t history[SHELL_HISTORY_LENGTH];
    shell_flags_t; // <- NOT PORTABLE - anonymous member struct via typedef
} shell_t;

extern shell_t shell;

#endif