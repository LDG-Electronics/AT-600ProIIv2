#ifndef _SHELL_INTERNALS_H_
#define _SHELL_INTERNALS_H_

/* ************************************************************************** */

#include "shell_config.h"

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

/* -------------------------------------------------------------------------- */

extern line_t shell;

/* ************************************************************************** */

#include "shell_command_processing.h"
#include "shell_cursor.h"
#include "shell_history.h"
#include "shell_sequences.h"

/* ************************************************************************** */

#endif