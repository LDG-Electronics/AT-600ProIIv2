#ifndef HARDWARE_H
#define	HARDWARE_H

/* ************************************************************************** */

#define _XTAL_FREQ 64000000

/* ************************************************************************** */

// Button stuff

// Button port accessors
#define TUNE_BUTTON !PORTAbits.RA0
#define FUNC_BUTTON !PORTAbits.RA1
#define ANT_BUTTON !PORTAbits.RA2
#define CUP_BUTTON !PORTAbits.RA3
#define CDN_BUTTON !PORTAbits.RA4
#define LUP_BUTTON !PORTAbits.RA5
#define LDN_BUTTON !PORTAbits.RA6

/* ************************************************************************** */


// Front Panel bitbang SPI
#define FP_CLOCK_PIN LATAbits.LATA6 //
#define FP_DATA_PIN LATCbits.LATC5 // 
#define FP_STROBE_PIN LATCbits.LATC4 //
 
// Front Panel LEDs
#define BYPASS_LED LATCbits.LATC3 // 
#define ANT_LED LATBbits.LATB1 // 

// Frequency Counter input
#define FREQ_PIN PORTEbits.RE0

//
#define BITBANG_PIN PORTCbits.RC6

/* ************************************************************************** */

extern void processor_init(void);
extern void startup(void);
extern void shutdown(void);

#endif	/* HARDWARE_H */

