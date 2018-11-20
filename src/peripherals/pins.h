#ifndef _PINS_H_
#define _PINS_H_

/* ************************************************************************** */

#define DEVELOPMENT

/* ************************************************************************** */

#define PORT(port, pin) PORT##port##bits.R##port##pin
#define LATCH(port, pin) LAT##port##bits.LAT##port##pin

/* ************************************************************************** */
// Button stuff
// todo comment about active low
#define POWER_BUTTON_PIN !PORT(A, 3)
#define CDN_BUTTON_PIN !PORT(A, 4)
#define LUP_BUTTON_PIN !PORT(A, 5)
#define CUP_BUTTON_PIN !PORT(B, 2)
#define FUNC_BUTTON_PIN !PORT(B, 4)
#define LDN_BUTTON_PIN !PORT(B, 5)

#ifdef DEVELOPMENT
#define ANT_BUTTON_PIN !PORT(E, 2)
#define TUNE_BUTTON_PIN !PORT(E, 1)
#else
#define ANT_BUTTON_PIN !PORT(B, 6)
#define TUNE_BUTTON_PIN !PORT(B, 7)
#endif

/* -------------------------------------------------------------------------- */
// Front Panel LEDs
// Bargraph bitbang SPI - Unused since the completion of the SPI driver
#define FP_CLOCK_PIN LATCH(A, 6)
#define FP_DATA_PIN LATCH(C, 5)
#define FP_STROBE_PIN LATCH(C, 4)

// Status LED pins
#define POWER_LED_PIN LATCH(A, 2)
#define BYPASS_LED_PIN LATCH(C, 3)
#define ANT_LED_PIN LATCH(B, 1)

/* -------------------------------------------------------------------------- */
// RF Sensor
#ifdef DEVELOPMENT
#define FREQ_PIN PORT(E, 0) // frequency counter
#else
#define FREQ_PIN PORT(E, 3) // frequency counter
#endif

#define FWD_PIN PORT(A, 0) // forward power
#define REV_PIN PORT(A, 1) // reverse power

// External Interrupts
#define RF_INT_PIN PORT(B, 3)    // rf interrupt - unused
#define INTERRUPT_PIN PORT(B, 0) // front panel button interrupt - unused

// Radio key line - used with IC-PAC or YT-PAC-1200 cable to key radio
#define RADIO_CMD_PIN LATCH(A, 7)

// Relay driver bitbang SPI
#define RELAY_CLOCK_PIN LATCH(C, 0)
#define RELAY_DATA_PIN LATCH(C, 1)
#define RELAY_STROBE_PIN LATCH(C, 2)

// Meter port - Unused since the completion of the UART driver
#define METER_TX_PIN LATCH(C, 6)
#define METER_RX_PIN PORT(C, 7)

// Serial debug
#define DEBUG_TX_PIN LATCH(D, 2)
#define DEBUG_RX_PIN PORT(D, 1)

/* ************************************************************************** */

void pins_init(void);

#endif /* _PINS_H_ */