#ifndef _SH_POLY_H_
#define _SH_POLY_H_

#include <stdint.h>

/* ************************************************************************** */

extern void shell_poly(int argc, char **argv);

#define SH_POLY                                                                \
    { shell_poly, "poly" }

/* ************************************************************************** */

#endif // _SH_POLY_H_