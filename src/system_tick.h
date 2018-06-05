#ifndef _SYSTEM_TICK_H_
#define _SYSTEM_TICK_H_

/* ************************************************************************** */

extern void systick_init(void);

/* -------------------------------------------------------------------------- */

#define print_current_time() printf("currentTime: %ld ", systick_read())

extern uint24_t systick_read(void);

#endif