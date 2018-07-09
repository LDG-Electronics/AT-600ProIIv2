#ifndef _SHELL_H_
#define _SHELL_H_

/* ************************************************************************** */

#include "shell_config.h"

// Type definition for all the programs invoked by the shell (function pointer)
typedef int (*shell_program_t) (int, char **);

/* ************************************************************************** */
extern void shell_init(void);

/*	@brief Registers a command with the command line library

 	Registers a command on the available command list. The name of the command
 	and the function that implements it's functionality should be provided.

 	@param program The type shell_program_t is a pointer to a function
 	that is executed when the written command matches the associated name

 	@param string A string containing the name of the command to be registered.

 	@return Returns true if command was successfully added to the command list,
 	or returns false if something fails (no more commands can be registered).
*/
extern bool shell_register(shell_program_t program, const char *string);

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