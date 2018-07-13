#ifndef _SHELL_CURSOR_H_
#define _SHELL_CURSOR_H_

/* ************************************************************************** */

// move cursor n spaces, positive moves right, negative moves left
extern void move_cursor(int8_t distance);

// move cursor to the desired absolute position
extern void move_cursor_to(uint8_t position);

/* -------------------------------------------------------------------------- */

// clear the current line and reprint it from line
extern void draw_line(line_t *line);

// clear the current line on screen and reprint it from the linebuffer
extern void redraw_current_line(void);

//
extern void insert_char_at_cursor(char currentChar);

//
extern void remove_char_at_cursor(void);

#endif