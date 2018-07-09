#ifndef _SHELL_H_
#define _SHELL_H_

/* ************************************************************************** */

#include "shell_config.h"



/* ************************************************************************** */
extern void shell_init(void);

/*	Main Shell processing loop
 	This function implements the main functionality of the command line interface
 	this function should be called frequently so it can handle the input from the
 	data stream.
*/
extern void shell_update(void);

/* -------------------------------------------------------------------------- */

// print all registered commands
extern void shell_print_commands(void);

// print error messages to console
extern void shell_print_error(int error, const char *field);

#endif