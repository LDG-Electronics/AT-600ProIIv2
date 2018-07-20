#ifndef _SHELL_COMMAND_PROCESSOR_H_
#define _SHELL_COMMAND_PROCESSOR_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "shell_internals.h"

/* ************************************************************************** */

extern void shell_register_command(shell_program_t program, const char *command,
                                   const char *usage);

/* ************************************************************************** */

// setup
extern void shell_commands_init(void);

// print all registered commands
extern void shell_print_commands(void);

extern void process_shell_command(void);

#endif
