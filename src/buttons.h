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
    provide some level of software debounce with hysteresis.
*/

/* ************************************************************************** */

// This enum should contain one entry for every front panel button.
typedef enum {TUNE, FUNC, CUP, CDN, LUP, LDN, ANT, POWER} buttonName_t;

// This pointer can refer to one of the button checking functions
typedef uint8_t (*button_check_t)(buttonName_t);

/* ************************************************************************** */

// Setup
extern void buttons_init(void);
extern void buttons_stop(void);

// Returns 1 if any button is current being held
extern uint8_t get_buttons(void);

/* -------------------------------------------------------------------------- */

/*  Notes on button states

    This button subsystem recognizes 4 distinct button states:
    pressed
    released
    down
    up

    The first two states represent the rising or falling edge of exactly when
    the user presses or released a button.

    The last two states represent when a button is either being held constantly
    or not being touched at all.
*/

// Returns a 1 if any of the listed buttons is experiencing the specified state
extern uint8_t check_multiple_buttons(button_check_t btn_is_xxx, uint8_t numberOfButtons, ...);

// Returns 1 if the specified button is experiencing a falling edge
extern uint8_t btn_is_pressed(buttonName_t buttonName);

// Returns 1 if the specified button is experiencing a rising edge
extern uint8_t btn_is_released(buttonName_t buttonName);

// Returns 1 if the specified button has been down for 8 samples
extern uint8_t btn_is_down(buttonName_t buttonName);

// Returns 1 if the specified button has been up for 8 samples
extern uint8_t btn_is_up(buttonName_t buttonName);

#endif