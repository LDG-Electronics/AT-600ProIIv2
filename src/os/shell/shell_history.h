#ifndef _SHELL_HISTORY_H_
#define _SHELL_HISTORY_H_

/* ************************************************************************** */

extern void shell_history_init(void);

extern void shell_history_wipe(void);

/* -------------------------------------------------------------------------- */

// push the current line onto the history stack
extern void shell_history_push(void);
extern void shell_history_show_older(void);
extern void shell_history_show_newer(void);

#endif