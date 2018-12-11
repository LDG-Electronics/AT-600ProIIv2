#ifndef _SH_RELAYS_H_
#define _SH_RELAYS_H_

#include <stdint.h>

/* ************************************************************************** */

extern void shell_relays(int argc, char **argv);

#define SH_RELAYS                                                              \
    { shell_relays, "relays" }

/* ************************************************************************** */

#endif // _SH_RELAYS_H_