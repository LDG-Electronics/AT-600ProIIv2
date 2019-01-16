#ifndef _SH_STATUS_H_
#define _SH_STATUS_H_

#include <stdint.h>

/* ************************************************************************** */

extern void sh_status(int argc, char **argv);

#define SH_STATUS                                                                 \
    { sh_status, "status" }

/* ************************************************************************** */

#endif // _SH_STATUS_H_