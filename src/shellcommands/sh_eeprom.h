#ifndef _SH_EEPROM_H_
#define _SH_EEPROM_H_

#include <stdint.h>

/* ************************************************************************** */

extern void shell_eeprom(int argc, char **argv);

#define SH_EEPROM                                                              \
    { shell_eeprom, "eeprom" }

/* ************************************************************************** */

#endif // _SH_EEPROM_H_