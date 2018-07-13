#ifndef _SHELL_COMMAND_PROCESSING_H_
#define _SHELL_COMMAND_PROCESSING_H_

/* ************************************************************************** */
// Shell command stuff

// Type definition for all the programs invoked by the shell (function pointer)
typedef int (*shell_program_t)(int, char **);

extern void shell_register_command(shell_program_t program, const char *command,
                           const char *usage);

/* ************************************************************************** */

// setup
extern void shell_commands_init(void) ;

// print all registered commands
extern void shell_print_commands(void);

extern void process_shell_command(void);

#endif
