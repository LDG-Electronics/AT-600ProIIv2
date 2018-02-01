#ifndef HARDWARE_H
#define	HARDWARE_H

/* ************************************************************************** */

#define _XTAL_FREQ 64000000

/* ************************************************************************** */
// Hardware platforms
// #define YT1200
#define Z11PROII
// #define AT100PROII

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

// User Interface - YT-1200
// #define LED_PIN LATBbits.LATB3 // output; YT-1200 has one LED
// #define BUTTON_PIN PORTBbits.RB0 // input; YT-1200 has one button

// Front Panel bitbang SPI - AT-100ProII
#define FP_CLOCK_PIN LATCbits.LATC4 //
#define FP_DATA_PIN LATCbits.LATC5 // 
#define FP_STROBE_PIN LATCbits.LATC1 //
 
// Front Panel LEDs - AT-100ProII
#define BYPASS_LED LATBbits.LATB4 // 
#define ANT_LED LATBbits.LATB5 // 

// LED SPI pins
#define CLOCK_PIN LATCbits.LATC0 //
#define DATA_PIN LATCbits.LATC1 //

// Bitbang UART, used for serial debug
// #define BITBANG_PIN LATCbits.LATC7 // 
#define BITBANG_PIN LATFbits.LATF6 // 


// Frequency Counter input
#define FREQ_PIN PORTEbits.RE0

// Z-11 Front Panel LEDS
#define LED_A LATBbits.LATB5
#define LED_B LATAbits.LATA7
#define LED_C LATBbits.LATB4
#define LED_D LATCbits.LATC1

// Z-11 Power Hold pin
#define POWER_HOLD LATAbits.LATA5

/* ************************************************************************** */

extern void processor_init(void);
extern void startup(void);
extern void shutdown(void);

#endif	/* HARDWARE_H */

