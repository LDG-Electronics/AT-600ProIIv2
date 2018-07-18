#ifndef _SHELL_UTILS_H_
#define _SHELL_UTILS_H_

/* ************************************************************************** */

// terminal control escape sequences
#define reset_text_attributes() print("\033[0m")

// clear the entire screen and move the cursor to 0,0
#define term_reset_screen() print("\033[2J")

// move the cursor to the given position
#define term_cursor_set(x, y) print("\033[##x##;##y##H")

// Move the cursor left a ton. The terminal won't let it go past the edge.
#define term_cursor_home() print("\033[100D")

// Move the cursor in some direction
#define term_cursor_up(distance) print("\033[##distance##A")
#define term_cursor_down(distance) print("\033[##distance##B")
#define term_cursor_right(distance) print("\033[##distance##C")
#define term_cursor_left(distance) print("\033[##distance##D")


#endif
