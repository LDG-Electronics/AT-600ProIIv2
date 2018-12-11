#ifndef _SH_FLASH_H_
#define _SH_FLASH_H_

#include <stdint.h>

/* ************************************************************************** */

extern void shell_flash(int argc, char **argv);

#define SH_FLASH                                                               \
    { shell_flash, "flash" }

/* ************************************************************************** */

#endif // _SH_FLASH_H_