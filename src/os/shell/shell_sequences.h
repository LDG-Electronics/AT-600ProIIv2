#ifndef _SHELL_SEQUENCES_H_
#define _SHELL_SEQUENCES_H_

/* ************************************************************************** */

// This macro can disable all the costly key name debug operations
// #define set_key_name(string)

// Call this every time we resolve a control keystroke
extern void set_key_name(const char * string);

/* -------------------------------------------------------------------------- */

extern void process_control_character(char currentChar);
extern void process_escape_sequence(char currentChar);

#endif