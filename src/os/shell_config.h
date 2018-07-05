#ifndef _SHELL_CONFIG_H_
#define	_SHELL_CONFIG_H_

/* ************************************************************************** */

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
#define SHELL_PROMPT_STRING "$ "

// Defines the maximum number of commands that can be registered
#define CONFIG_SHELL_MAX_COMMANDS 20

// Defines the maximum characters that the input buffer can accept
#define CONFIG_SHELL_MAX_INPUT 70

// Configures the maximum number of arguments per command tha can be accepted
#define CONFIG_SHELL_MAX_COMMAND_ARGS 10

// Defines the buffer for formatted string output from program memory
#define CONFIG_SHELL_FMT_BUFFER 70

#endif