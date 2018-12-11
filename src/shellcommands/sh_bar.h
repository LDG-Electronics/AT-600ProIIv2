#ifndef _SH_BAR_H_
#define _SH_BAR_H_

#include <stdint.h>

/* ************************************************************************** */

extern void shell_show_bargraphs(int argc, char **argv);

#define SH_BAR                                                          \
    { shell_show_bargraphs, "bar" }

/* ************************************************************************** */

#endif // _SH_BAR_H_