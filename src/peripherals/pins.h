#ifndef _PINS_H_
#define _PINS_H_

/* ************************************************************************** */
// Button stuff
#define POWER_BUTTON_PIN !PORTAbits.RA3
#define CDN_BUTTON_PIN !PORTAbits.RA4
#define LUP_BUTTON_PIN !PORTAbits.RA5
#define CUP_BUTTON_PIN !PORTBbits.RB2
#define FUNC_BUTTON_PIN !PORTBbits.RB4
#define LDN_BUTTON_PIN !PORTBbits.RB5
// #define ANT_BUTTON_PIN !PORTBbits.RB6
#define ANT_BUTTON_PIN !PORTEbits.RE2
// #define TUNE_BUTTON_PIN !PORTBbits.RB7
#define TUNE_BUTTON_PIN !PORTEbits.RE1

/* -------------------------------------------------------------------------- */
// Front Panel LEDs
// Bargraph bitbang SPI - Unused since the completion of the SPI driver
#define FP_CLOCK_PIN LATAbits.LATA6
#define FP_DATA_PIN LATCbits.LATC5
#define FP_STROBE_PIN LATCbits.LATC4

// Status LED pins
#define POWER_LED_PIN LATAbits.LATA2
#define BYPASS_LED_PIN LATCbits.LATC3
#define ANT_LED_PIN LATBbits.LATB1

/* -------------------------------------------------------------------------- */
// RF Sensor
// #define FREQ_PIN PORTEbits.RE3 // frequency counter
#define FREQ_PIN PORTEbits.RE0 // frequency counter
#define FWD_PIN PORTAbits.RA0  // forward power
#define REV_PIN PORTAbits.RA1  // reverse power

// External Interrupts
#define RF_INT_PIN PORTBbits.RB3    // rf interrupt - unused
#define INTERRUPT_PIN PORTBbits.RB0 // front panel button interrupt - unused

// Radio key line - used with IC-PAC or YT-PAC-1200 cable to key radio
#define RADIO_CMD_PIN PORTAbits.RA7

// Relay driver bitbang SPI
#define RELAY_CLOCK_PIN LATCbits.LATC0
#define RELAY_DATA_PIN LATCbits.LATC1
#define RELAY_STROBE_PIN LATCbits.LATC2

// Meter port - Unused since the completion of the UART driver
#define METER_TX_PIN LATCbits.LATC6
#define METER_RX_PIN PORTCbits.RC7

// Serial debug
#define DEBUG_TX_PIN LATDbits.LATD2
#define DEBUG_RX_PIN PORTDbits.RD1

/* ************************************************************************** */

void pins_init(void);

#endif /* _PINS_H_ */