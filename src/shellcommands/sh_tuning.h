#ifndef _SH_TUNING_H_
#define _SH_TUNING_H_

#include <stdint.h>

/* ************************************************************************** */

extern void tune(int argc, char **argv);
extern void mem(int argc, char **argv);

#define SH_TUNE                                                                \
    { tune, "tune" }
#define SH_MEM                                                                 \
    { mem, "mem" }

/* ************************************************************************** */

#endif