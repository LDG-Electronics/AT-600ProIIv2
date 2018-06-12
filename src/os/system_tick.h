#ifndef _SYSTEM_TICK_H_
#define _SYSTEM_TICK_H_

/* ************************************************************************** */

extern void systick_init(void);

/* -------------------------------------------------------------------------- */

extern uint24_t systick_read(void);

extern uint24_t systick_elapsed_time(uint24_t startTime);

extern void systick_delay(uint16_t mseconds);

/* ************************************************************************** */

// This block should be executed whenever the system is engaged in a blocking
// loop. 
extern void system_idle_block(void);

#endif