#ifndef _SHELL_H_
#define _SHELL_H_

/* ************************************************************************** */

#define SHELL_RET_SUCCESS			0
#define SHELL_RET_FAILURE			1
#define SHELL_RET_IOPENDING			-1

/* -------------------------------------------------------------------------- */

// Type definition for all the programs invoked by the shell (function pointer)
typedef int (*shell_program_t) (int, char **);

// Errors that could be printed to the console
enum shell_errors {
	/** There are missing arguments for the command */
	E_SHELL_ERR_ARGCOUNT = 0,
	/** The program received an argument that is out of range */
	E_SHELL_ERR_OUTOFRANGE,
	/** The program received an argument with a value different than expected */
	E_SHELL_ERR_VALUE,
	/** Invalid action requested for the current state */
	E_SHELL_ERR_ACTION,
	/** Cannot parse the user input */
	E_SHELL_ERR_PARSE,
	/** Cannot access storage device or memory device */
	E_SHELL_ERR_STORAGE,
	/** IO device error caused program interruption */
	E_SHELL_ERR_IO,
	/** Other kinds of errors */
	E_SHELL_ERROR_UNKNOWN,
};

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
extern void shell_task(void);

/* -------------------------------------------------------------------------- */

// print all registered commands
extern void shell_print_commands(void);

// print error messages to console
extern void shell_print_error(int error, const char *field);

// Prints a null terminated string to the terminal
extern void shell_print(const char *string);
extern void shell_println(const char *string);

#endif