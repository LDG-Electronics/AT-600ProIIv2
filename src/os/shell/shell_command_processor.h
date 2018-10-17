#ifndef _SHELL_COMMAND_PROCESSOR_H_
#define _SHELL_COMMAND_PROCESSOR_H_

#include "shell.h"

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
