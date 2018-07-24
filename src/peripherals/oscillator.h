#ifndef _OSCILLATOR_H_
#define _OSCILLATOR_H_

/* ************************************************************************** */

// Oscillator stuff
#define FOSC_32_MHZ 32000000
#define FOSC_64_MHZ 64000000
#define CLOCK_FREQ FOSC_64_MHZ

/* ************************************************************************** */

extern void oscillator_init(void);

#endif