#ifndef _SHELL_COMMAND_PROCESSOR_H_
#define _SHELL_COMMAND_PROCESSOR_H_

#include "shell.h"

/* ************************************************************************** */

/*  shell_command_t

    callback
    A pointer to the shell command body.

    command
    A pointer to string that represents the command that needs to be typed
*/
typedef struct {
    shell_program_t callback;
    const char *command;
} shell_command_t;

/* ************************************************************************** */

extern void shell_register_command(shell_program_t program,
                                   const char *command);

/* ************************************************************************** */

// setup
extern void shell_commands_init(void);

// print all registered commands
extern void shell_print_commands(void);

extern void process_shell_command(void);

#endif
