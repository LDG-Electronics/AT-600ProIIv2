#ifndef _SHELL_COMMAND_PROCESSOR_H_
#define _SHELL_COMMAND_PROCESSOR_H_

#include <stdint.h>

/* ************************************************************************** */

// Attempt to parse the shell input and excute the matching command
extern int8_t process_shell_command(char *string);

#endif
