#ifndef _SH_NONVOLATILE_MEMORY_H_
#define _SH_NONVOLATILE_MEMORY_H_

#include <stdint.h>

/* ************************************************************************** */

extern void shell_eeprom(int argc, char **argv);
extern void shell_flash(int argc, char **argv);

#define SH_EEPROM                                                              \
    { shell_eeprom, "eeprom" }
#define SH_FLASH                                                               \
    { shell_flash, "flash" }

/* ************************************************************************** */

#endif