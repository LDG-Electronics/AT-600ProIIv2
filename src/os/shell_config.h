#ifndef _SHELL_CONFIG_H_
#define	_SHELL_CONFIG_H_

/* ************************************************************************** */

#define SHELL_RET_SUCCESS			0
#define SHELL_RET_FAILURE			1
#define SHELL_RET_IOPENDING			-1

/* ************************************************************************** */

#define SHELL_VERSION_STRING "\r\nTuneOS Shell v0.1"

// #define SHELL_PROMPT_COLOR "\033[32m"
// #define DEFAULT_COLOR "\033[0m"
#define SHELL_PROMPT_STRING "\033[32m$ \033[0m"
// #define SHELL_PROMPT_STRING "$ "

// Defines the maximum number of commands that can be registered
#define MAXIMUM_NUM_OF_SHELL_COMMANDS 20

// Defines the maximum characters that the input buffer can accept
#define CONFIG_SHELL_MAX_INPUT 70
#define SHELL_BUFFER_LENGTH 70

// Configures the maximum number of arguments per command tha can be accepted
#define CONFIG_SHELL_MAX_COMMAND_ARGS 10

// Defines the buffer for formatted string output from program memory
#define CONFIG_SHELL_FMT_BUFFER 70

#endif