#ifndef _CONTROLS_H_
#define _CONTROLS_H_

/* ************************************************************************** */

// Button timing constants, in 1ms increments.
#define BTN_PRESS_DEBOUNCE  10
#define BTN_PRESS_SHORT     400
#define BTN_PRESS_MEDIUM    2500
#define BTN_PRESS_LONG      10000

/* ************************************************************************** */

//
extern void short_tune_release(void);
extern void medium_tune_release(void);
extern void long_tune_release(void);

// Front panel control handlers
extern void toggle_bypass(void);
extern void toggle_auto(void);
extern void toggle_hiloz(void);
extern void manual_store(void);

// 
extern void mode_thresh(void);
extern void mode_func(void);

#endif