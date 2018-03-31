#ifndef HARDWARE_H
#define	HARDWARE_H

/* ************************************************************************** */

#define _XTAL_FREQ 64000000

/* ************************************************************************** */

// Button stuff

// Button port accessors
#define POWER_BUTTON !PORTAbits.RA3
#define CDN_BUTTON !PORTAbits.RA4
#define LUP_BUTTON !PORTAbits.RA5
#define CUP_BUTTON !PORTBbits.RB2
#define FUNC_BUTTON !PORTBbits.RB4
#define LDN_BUTTON !PORTBbits.RB5
#define ANT_BUTTON !PORTBbits.RB6
#define TUNE_BUTTON !PORTBbits.RB7

/* ************************************************************************** */

// User interface
// Front Panel bitbang SPI
#define FP_CLOCK_PIN LATAbits.LATA6 //
#define FP_DATA_PIN LATCbits.LATC5 // 
#define FP_STROBE_PIN LATCbits.LATC4 //
 
// Front Panel LEDs
#define POWER_LED_PIN LATAbits.LATA2  
#define BYPASS_LED_PIN LATCbits.LATC3 // 
#define ANT_LED_PIN LATBbits.LATB1 // 

/* -------------------------------------------------------------------------- */

// RF Sensor
#define FREQ_PIN PORTEbits.RE0 // frequency counter
#define FWD_PIN PORTAbits.RA0 // forward power
#define REV_PIN PORTAbits.RA1 // reverse power

#define RF_INT_PIN PORTBbits.RB3 // rf interrupt - unused
#define INTERRUPT_PIN PORTBbits.RB0 // front panel button interrupt - unused

//
#define RADIO_CMD_PIN PORTAbits.RA7

// Meter port
#define METER_TX_PIN LATCbits.LATC6
#define METER_RX_PIN PORTCbits.RC7

//
#define BITBANG_PIN LATCbits.LATC6

/* ************************************************************************** */

extern void processor_init(void);
extern void startup(void);
extern void shutdown(void);

#endif	/* HARDWARE_H */

