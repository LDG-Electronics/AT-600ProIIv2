#ifndef _SHELL_H_
#define _SHELL_H_

/* ************************************************************************** */

#include "shell_config.h"
#include <stdint.h>

/* -------------------------------------------------------------------------- */

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

// Type definition for all the programs invoked by the shell (function pointer)
typedef int (*shell_program_t)(int, char **);

/* ************************************************************************** */

// setup
extern void shell_init(void);

extern void shell_set_program_callback(shell_program_t callback);

/*	Main Shell processing

    This function implements the main functionality of the command line
    interface this function should be called frequently so it can handle the
    input from the data stream.
*/
extern void shell_update(void);

#endif