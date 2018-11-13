#ifndef _SHELL_COMMANDS_H_
#define _SHELL_COMMANDS_H_

#include "os/log.h"

/* ************************************************************************** */

extern void shell_show_bargraphs(int argc, char **argv);
extern void calibration_packet(int argc, char **argv);
extern void fwd(int argc, char **argv);
extern void rev(int argc, char **argv);
extern void tune(int argc, char **argv);
extern void mem(int argc, char **argv);
extern void shell_eeprom(int argc, char **argv);
extern void shell_flash(int argc, char **argv);

#define SHELL_COMMANDS                                                         \
    {shell_show_bargraphs, "bar"}, {calibration_packet, "cal"}, {fwd, "fwd"},  \
        {rev, "rev"}, {tune, "tune"}, {mem, "mem"}, {logedit, "logedit"},      \
        {shell_eeprom, "eeprom"}, {                                            \
        shell_flash, "flash"                                                   \
    }

/* ************************************************************************** */

#endif // _SHELL_COMMANDS_H_