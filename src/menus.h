#ifndef _MENUS_H_
#define _MENUS_H_

/* ************************************************************************** */

// This block should be executed whenever the system is engaged in a blocking
// loop. 
extern void system_idle_block(void);

/* ************************************************************************** */
// Nested state machine handlers for front panel buttons

// relay increment buttons
extern void relay_button_hold(void);

// other buttons
extern void tune_hold(void);
extern void func_hold(void);
extern void ant_hold(void);
extern void power_hold(void);

#endif