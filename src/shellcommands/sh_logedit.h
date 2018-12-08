#ifndef _SH_LOGEDIT_H_
#define _SH_LOGEDIT_H_

#include <stdint.h>

/* ************************************************************************** */

// shell program to edit registered logging levels
void logedit(int argc, char **argv);
int8_t logedit_callback(char currentChar);

#define SH_LOGEDIT                                                             \
    { logedit, "logedit" }

/* ************************************************************************** */

#endif