#ifndef SHELL_COMMANDS_H
#define SHELL_COMMANDS_H

/* ************************************************************************** */

// Shell command standard return values
#define SHELL_RET_SUCCESS 0
#define SHELL_RET_FAILURE 1

/* ************************************************************************** */

extern void register_all_shell_commands(void);

/* ************************************************************************** */

// from shell.c
extern int shell_help(int argc, char **argv);
extern int shell_arg_test(int argc, char **argv);

// general purpose parameter touching
extern int shell_get_param(int argc, char **argv);
extern int shell_set_param(int argc, char **argv);

// from RF_sensor.c
extern int shell_get_RF(int argc, char **argv);

// from display.c
extern int shell_show_bargraphs(int argc, char **argv);

#endif