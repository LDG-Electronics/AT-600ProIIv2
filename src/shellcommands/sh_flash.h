#ifndef _SH_FLASH_H_
#define _SH_FLASH_H_

#include "../peripherals/nonvolatile_memory.h"
#include <stdint.h>

/* ************************************************************************** */

extern NVM_address_t decode_address(char *string);
extern int16_t decode_data(char *string);
extern void write_single_byte(NVM_address_t address, uint8_t newData);

extern void shell_flash(int argc, char **argv);

#define SH_FLASH                                                               \
    { shell_flash, "flash" }

/* ************************************************************************** */

#endif // _SH_FLASH_H_