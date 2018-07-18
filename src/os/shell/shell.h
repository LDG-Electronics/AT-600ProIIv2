#ifndef _SHELL_H_
#define _SHELL_H_

/* ************************************************************************** */

// Type definition for all the programs invoked by the shell (function pointer)
typedef int (*shell_program_t)(int, char **);

/* ************************************************************************** */

// setup
extern void shell_init(void);

extern void shell_set_program_callback(shell_program_t callback);

/*	Main Shell processing 
 	This function implements the main functionality of the command line interface
 	this function should be called frequently so it can handle the input from the
 	data stream.
*/
extern void shell_update(void);

#endif