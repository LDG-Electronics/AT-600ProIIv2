#ifndef _BUTTONS_H_
#define _BUTTONS_H_

/* ************************************************************************** */

/*  Notes on using the button processing subsystem_flags.
    
    This module is designed as a publisher/subscriber model, with two loosely
    coupled components.
    
    The publisher polls the physical buttons at a regular interval, 5ms is
    a typical value.  The publisher then records the current state of each
    button to a ledger or history.  This works best when it happens 
    asynchronously with the subscriber half of the module, such as in an 
    interrupt service routine triggered by a timer overflow, or a dedicated
    RTOS thread.
    
    The subscriber half is applied wherever it is desired to check on the status
    of an external input.  A call to one of the btn_is_xxxx() functions will
    look at the history that's been recorded by the publisher and return a yes
    or no, based on whether the history matches a predetermined pattern.  The
    specific pattern is different for each that is being evaluated, but they all
    provide some level of software debouncing with hysteresis.
*/

/* ************************************************************************** */

#define FRONT_PANEL_BUTTONS 8

// This enum should contain one entry for every front panel button.
typedef enum {TUNE, FUNC, CUP, CDN, LUP, LDN, ANT, POWER} buttonName_t;

// This is only public so it can be accessed by the ISR.  If not ISR, no touchy.
extern volatile uint8_t buttons[FRONT_PANEL_BUTTONS];

/* ************************************************************************** */

// Setup
extern void buttons_init(void);

// Deprecated
extern uint8_t get_buttons(void);
extern uint8_t get_buttons2(void);

// button handley-ness
extern uint8_t btn_is_pressed(buttonName_t buttonName);
extern uint8_t btn_is_released(buttonName_t buttonName);
extern uint8_t btn_is_down(buttonName_t buttonName);
extern uint8_t btn_is_up(buttonName_t buttonName);

#endif