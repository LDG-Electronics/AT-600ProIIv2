#ifndef _CONTROLS_H_
#define _CONTROLS_H_

/* ************************************************************************** */

// Button timing constants, in 1ms increments.
#define BTN_PRESS_DEBOUNCE  10
#define BTN_PRESS_SHORT     400
#define BTN_PRESS_MEDIUM    2500
#define BTN_PRESS_LONG      10000

/* ************************************************************************** */

extern void cup_hold(void);
extern void cdn_hold(void);
extern void lup_hold(void);
extern void ldn_hold(void);

extern void power_hold(void);
extern void tune_hold(void);

extern void func_hold(void);
extern void func_release(void);

#endif