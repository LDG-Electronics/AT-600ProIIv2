#ifndef _SHELL_COMMAND_BUILTINS_H_
#define _SHELL_COMMAND_BUILTINS_H_

/* ************************************************************************** */

extern int shell_help(int argc, char **argv);
extern int shell_arg_test(int argc, char **argv);

#define BUILTIN_COMMANDS {shell_help, "help"}, {shell_arg_test, "test"},


#endif