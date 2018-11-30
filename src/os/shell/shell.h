#ifndef _SHELL_H_
#define _SHELL_H_

/* ************************************************************************** */

#include "../../peripherals/pps.h"
#include "shell_config.h"
#include <stdint.h>

/* -------------------------------------------------------------------------- */

/*  shell_line_t

    A shell line contains a char buffer that is SHELL_MAX_LENGTH long, and two
    variables to keep track of the current length of the buffer and the current
    location of the cursor.
*/
typedef struct {
    char buffer[SHELL_MAX_LENGTH];
    uint8_t length;
    uint8_t cursor;
} shell_line_t;

extern shell_line_t shell;

/* -------------------------------------------------------------------------- */

// Type definition for all the programs invoked by the shell (function pointer)
typedef void (*shell_program_t)(int, char **);

// type definition
typedef int8_t (*shell_callback_t)(char);

/* ************************************************************************** */

// setup
extern void shell_init(pps_input_t rxPin, pps_output_t *txPin);

// Registers a
extern void shell_register_callback(shell_callback_t callback);

/*	Main Shell processing

    This function implements the main functionality of the command line
    interface this function should be called frequently so it can handle the
    input from the data stream.
*/
extern void shell_update(void);

#endif // _SHELL_H_