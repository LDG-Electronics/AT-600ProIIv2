#ifndef _HARDWARE_H_
#define	_HARDWARE_H_

/* ************************************************************************** */
// Oscillator stuff
#define FOSC_32_MHZ 32000000
#define FOSC_64_MHZ 64000000
#define CLOCK_FREQ FOSC_64_MHZ

// AT-600ProII has 8 front panel buttons
#define NUMBER_OF_BUTTONS 8

// This enum should contain one entry for every front panel button.
typedef enum {TUNE, FUNC, CUP, CDN, LUP, LDN, ANT, POWER} buttonName_t;

/* ************************************************************************** */

// Macros to control interrupt system
#define begin_critical_section() (INTCON0bits.GIE = 0)
#define end_critical_section() (INTCON0bits.GIE = 1)

extern void startup(void);
extern void shutdown(void);

#endif	/* _HARDWARE_H_ */