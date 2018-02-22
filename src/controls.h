#ifndef _CONTROLS_H_
#define _CONTROLS_H_

/* ************************************************************************** */

// Button timing constants, in 1ms increments.
#define BTN_PRESS_DEBOUNCE  20
#define BTN_PRESS_SHORT     350
#define BTN_PRESS_MEDIUM    2000
#define BTN_PRESS_LONG      10000

/* ************************************************************************** */

extern void cup_hold(void);
extern void cdn_hold(void);
extern void lup_hold(void);
extern void ldn_hold(void);

extern void tune_hold(void);
extern void func_hold(void);
extern void ant_hold(void);
extern void power_hold(void);

#endif