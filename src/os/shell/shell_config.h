#ifndef _SHELL_CONFIG_H_
#define _SHELL_CONFIG_H_

/* ************************************************************************** */
// Shell version 
#define SHELL_VERSION_STRING "\r\nTuneOS Shell v0.1"

#define SHELL_PROMPT_COLOR "\033[32m"
#define DEFAULT_COLOR "\033[0m"
#define SHELL_PROMPT "$ "

#define SHELL_PROMPT_STRING SHELL_PROMPT_COLOR SHELL_PROMPT DEFAULT_COLOR
// #define SHELL_PROMPT_STRING SHELL_PROMPT

/* -------------------------------------------------------------------------- */
// Core shell options

// Defines the maximum number of commands that can be registered
#define MAXIMUM_NUM_OF_SHELL_COMMANDS 20

// Defines the maximum characters that the input buffer can accept
#define SHELL_MAX_LENGTH 50

/* -------------------------------------------------------------------------- */
// Escape Sequence Processing options

// Configures the length of the buffer used to process escape sequences
#define ESCAPE_BUFFER_LENGTH 10

/* -------------------------------------------------------------------------- */
// Command processing options

// Configures the maximum number of arguments per command tha can be accepted
#define CONFIG_SHELL_MAX_COMMAND_ARGS 10

/* ************************************************************************** */
// Optional Features

/* -------------------------------------------------------------------------- */
// Shell history options

// Defines the number of lines stored in the shell history buffer
#define SHELL_HISTORY_LENGTH 16

#endif