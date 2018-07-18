#ifndef _SHELL_UTILS_H_
#define _SHELL_UTILS_H_

/* ************************************************************************** */

// terminal control escape sequences
#define reset_text_attributes() print("\033[0m")
#define term_reset_screen() print("\033[2J")

#define term_cursor_set(x, y) print("\033[##x##;##y##H")

#define term_cursor_home() print("\033[100D")
#define term_cursor_up(distance) print("\033[##distance##A")
#define term_cursor_down(distance) print("\033[##distance##B")
#define term_cursor_right(distance) print("\033[##distance##C")
#define term_cursor_left(distance) print("\033[##distance##D")


#endif
