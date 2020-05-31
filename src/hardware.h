#ifndef _HARDWARE_H_
#define _HARDWARE_H_

/* ************************************************************************** */
// version information

/* [[[cog cog.outl(f'#define PRODUCT_NAME "{utils.project.name}"') ]]] */
#define PRODUCT_NAME "AT-600ProII"
/* [[[end]]] */

/* ************************************************************************** */
//! There's no way to prove that this list is up-to-date.
//! Search the entire project for "timer" or "timer.h" before claiming a timer.

// Timer registry
// Timer0 is used in the stopwatch module
// Timer1 is unused
// Timer2 is used for the systick module
// Timer3 is used in the frequency counter
// Timer4 is used in the frequency counter
// Timer5 is unused
// Timer6 is used for the button debounce subsystem

/* ************************************************************************** */
// Button stuff

// AT-600ProII has 8 front panel buttons
#define NUMBER_OF_BUTTONS 8

// This is an X Macro that is used to populate the button driver
#define BUTTON_LIST                                                            \
    X(TUNE)                                                                    \
    X(FUNC)                                                                    \
    X(CUP)                                                                     \
    X(CDN)                                                                     \
    X(LUP)                                                                     \
    X(LDN)                                                                     \
    X(ANT)                                                                     \
    X(POWER)

// What is the format of the button pin names?
/*  Buttons are wired with pullups to 5v, and are active-low. This helps prevent
    random extra button presses caused by the noisy, highRF environment.

    Consequently, inputs need to be inverted.
*/
#define BUTTON_PIN_NAME_FORMAT(NAME) !NAME##_BUTTON_PIN

/* ************************************************************************** */

extern void startup(void);
extern void shutdown(void);

#endif // _HARDWARE_H_