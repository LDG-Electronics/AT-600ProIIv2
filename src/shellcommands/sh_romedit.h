#ifndef _SH_ROMEDIT_H_
#define _SH_ROMEDIT_H_

#include <stdint.h>

/* ************************************************************************** */

// shell program to edit registered logging levels
void romedit(int argc, char **argv);
int8_t romedit_callback(char currentChar);

#define SH_ROMEDIT                                                             \
    { romedit, "romedit" }

/* ************************************************************************** */

#endif // _SH_ROMEDIT_H_