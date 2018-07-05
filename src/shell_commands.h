#ifndef SHELL_COMMANDS_H
#define SHELL_COMMANDS_H

/* ************************************************************************** */

extern void shell_commands_init(void);

/* -------------------------------------------------------------------------- */
/*  Shell commands

    function pointer type for shell callbacks
    typedef int (*shell_program_t) (int, char **)

    function signature for a shell command
    extern int shell_command(int argc, char** argv);

*/

// from RF_sensor.c
extern int shell_get_RF(int argc, char **argv);

// from display.c
extern int shell_show_bargraphs(int argc, char **argv);

// from relays.c
extern int shell_set_relays(int argc, char **argv);
extern int shell_check_relays(int argc, char **argv);

// from shell.c
extern int shell_help(int argc, char **argv);
extern int shell_test(int argc, char **argv);

#endif