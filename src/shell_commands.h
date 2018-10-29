#ifndef SHELL_COMMANDS_H
#define SHELL_COMMANDS_H

#include "os/log.h"

/* ************************************************************************** */

extern int shell_show_bargraphs(int argc, char **argv);
extern int calibration_packet(int argc, char **argv);
extern int poly(int argc, char **argv);
extern int fwd(int argc, char **argv);
extern int rev(int argc, char **argv);
extern int tune(int argc, char **argv);
extern int shell_flash(int argc, char **argv) ;

#define SHELL_COMMANDS                                                         \
    {shell_show_bargraphs, "bar"}, {calibration_packet, "cal"},                \
        {poly, "poly"}, {fwd, "fwd"}, {rev, "rev"}, {tune, "tune"},            \
        {program_logedit_begin, "logedit"}, {shell_flash, "flash"}

#endif