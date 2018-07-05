#ifndef _HARDWARE_H_
#define	_HARDWARE_H_

/* ************************************************************************** */
// Oscillator stuff
#define FOSC_32_MHZ 32000000
#define FOSC_64_MHZ 64000000
#define CLOCK_FREQ FOSC_64_MHZ

/* -------------------------------------------------------------------------- */
// Button stuff

// AT-600ProII has 8 front panel buttons
#define NUMBER_OF_BUTTONS 8

// This is an X Macro that is used to populate the button driver
#define BUTTON_LIST \
    X(TUNE)\
    X(FUNC)\
    X(CUP)\
    X(CDN)\
    X(LUP)\
    X(LDN)\
    X(ANT)\
    X(POWER)

/* ************************************************************************** */

// Macros to control interrupt system
#define begin_critical_section() (INTCON0bits.GIE = 0)
#define end_critical_section() (INTCON0bits.GIE = 1)

extern void startup(void);
extern void shutdown(void);

#endif	/* _HARDWARE_H_ */