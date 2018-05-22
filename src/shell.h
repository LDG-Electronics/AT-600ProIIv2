#ifndef _SHELL_H_
#define _SHELL_H_

/* ************************************************************************** */

typedef enum {
	SUCCESS = 0, 
	FAILURE = 1, 
	IOPENDING = -1
}shell_return_value_s;

#define SHELL_RET_SUCCESS			0
#define SHELL_RET_FAILURE			1
#define SHELL_RET_IOPENDING			-1

/* ************************************************************************** */

// ASCII helper macros
#define SHELL_ASCII_NUL				'\0'
#define SHELL_ASCII_BEL				'\a'
#define SHELL_ASCII_BS				'\b'
#define SHELL_ASCII_HT				'\t'
#define SHELL_ASCII_LF				'\n'
#define SHELL_ASCII_CR				'\r'
#define SHELL_ASCII_ESC				0x1B
#define SHELL_ASCII_DEL				0x7F
#define SHELL_ASCII_US				0x1F
#define SHELL_ASCII_SP				' '
#define SHELL_VT100_ARROWUP			'A'
#define SHELL_VT100_ARROWDOWN		'B'
#define SHELL_VT100_ARROWRIGHT		'C'
#define SHELL_VT100_ARROWLEFT		'D'

/* ************************************************************************** */

#define SHELL_VERSION_STRING "\r\nuShell 1.0.1"
#define SHELL_PROMPT_STRING "$"

// Defines the maximum number of commands that can be registered
#define CONFIG_SHELL_MAX_COMMANDS 20

// Defines the maximum characters that the input buffer can accept
#define CONFIG_SHELL_MAX_INPUT 70

// Configures the maximum number of arguments per command tha can be accepted
#define CONFIG_SHELL_MAX_COMMAND_ARGS 10

// Defines the buffer for formatted string output from program memory
#define CONFIG_SHELL_FMT_BUFFER 70

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

// print functions for printing to the serial port
extern void putch(const char data);
extern void print(const char *string);
extern void println(const char *string);

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