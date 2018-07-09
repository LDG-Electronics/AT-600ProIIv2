#ifndef _SHELL_CURSOR_H_
#define _SHELL_CURSOR_H_

/* ************************************************************************** */

extern void move_cursor(int8_t distance);

extern void move_cursor_to(uint8_t position);

extern void insert_char_at_cursor(char currentChar);

extern void remove_char_at_cursor(void);

#endif