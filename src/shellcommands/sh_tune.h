#ifndef _SH_TUNE_H_
#define _SH_TUNE_H_

#include <stdint.h>

/* ************************************************************************** */

extern void tune(int argc, char **argv);

#define SH_TUNE                                                                \
    { tune, "tune" }

/* ************************************************************************** */

#endif // _SH_TUNE_H_