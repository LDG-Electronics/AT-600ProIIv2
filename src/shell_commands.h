#ifndef SHELL_COMMANDS_H
#define SHELL_COMMANDS_H

/* ************************************************************************** */
// Shell command stuff

// Type definition for all the programs invoked by the shell (function pointer)
typedef int (*shell_program_t)(int, char **);

// Shell command standard return values
#define SHELL_RET_SUCCESS 0
#define SHELL_RET_FAILURE 1

// Shell command standard return values
typedef enum { SUCCESS, FAILURE } shell_return_t;

/* ************************************************************************** */

/*  shell command list

    This data structure is the central registry for shell commands.

    Individual commands should be defined in shell_commands.c and registered in
    shell_commands_init().

    The function signature of a shell command must be:
    int (*shell_program_t) (int, char **)
*/

/*  shell_command_t

    callback
    A pointer to the shell command body.

    command
    A pointer to string that represents the command that needs to be typed
*/
typedef struct {
    shell_program_t callback;
    const char *command;
    const char *usage;
} shell_command_t;

/*  command_list_t

    list
    An array of shell_command_t objects that stores all the registered shell
    commands.

    numOfRegisteredCommands
    The number of commands registered with the shell
*/
typedef struct commands {
    shell_command_t list[MAXIMUM_NUM_OF_SHELL_COMMANDS];
    uint8_t number;
} command_list_t;

extern command_list_t commands;

/* ************************************************************************** */

extern void shell_commands_init(void);

/* ************************************************************************** */

// from shell.c
extern int shell_help(int argc, char **argv);
extern int shell_arg_test(int argc, char **argv);

// from RF_sensor.c
extern int shell_get_RF(int argc, char **argv);

// from display.c
extern int shell_show_bargraphs(int argc, char **argv);

// from relays.c
extern int shell_set_relays(int argc, char **argv);
extern int shell_check_relays(int argc, char **argv);

#endif