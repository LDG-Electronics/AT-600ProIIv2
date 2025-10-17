#ifndef _PINS_H_
#define _PINS_H_

#include "peripherals/pps.h"
#include <stdbool.h>

/* ************************************************************************** */
/* [[[cog
    from codegen import fmt; import pins
    cog.outl(fmt(pins.pin_declarations()))
]]] */

// GPIO read functions
extern bool read_POWER_BUTTON_PIN(void);
extern bool read_CDN_BUTTON_PIN(void);
extern bool read_LUP_BUTTON_PIN(void);
extern bool read_CUP_BUTTON_PIN(void);
extern bool read_FUNC_BUTTON_PIN(void);
extern bool read_LDN_BUTTON_PIN(void);
extern bool read_FREQ_PIN(void);
extern bool read_ANT_BUTTON_PIN(void);
extern bool read_TUNE_BUTTON_PIN(void);

// GPIO write functions
extern void set_POWER_LED_PIN(bool value);
extern void set_RADIO_CMD_PIN(bool value);
extern void set_ANT_LED_PIN(bool value);
extern void set_RELAY_CLOCK_PIN(bool value);
extern void set_RELAY_DATA_PIN(bool value);
extern void set_RELAY_STROBE_PIN(bool value);
extern void set_BYPASS_LED_PIN(bool value);
extern void set_FP_STROBE_PIN(bool value);
extern void set_FP_DATA_PIN(bool value);
extern void set_FP_CLOCK_PIN(bool value);

// GPIO direction functions
// none

// Button stuff
#define NUMBER_OF_BUTTONS 8

// array of pointers to button reading functions
typedef bool (*button_function_t)(void);
extern button_function_t buttonFunctions[NUMBER_OF_BUTTONS];

// enum of button names
enum {
    POWER,
    CDN,
    LUP,
    CUP,
    FUNC,
    LDN,
    ANT,
    TUNE,
} button_names;

// PPS Pin initialization macros
#ifdef DEVELOPMENT
    #define PPS_DEBUG_RX_PIN PPS_INPUT(B, 6)
#endif
#ifdef DEVELOPMENT
    #define PPS_DEBUG_TX_PIN PPS_OUTPUT(B, 7)
#endif
#ifdef DEVELOPMENT
    #define PPS_FREQ_PIN PPS_INPUT(F, 0)
#else
    #define PPS_FREQ_PIN PPS_INPUT(E, 3)
#endif

// ADC Channel Select macros
#define ADC_FWD_PIN 0
#define ADC_REV_PIN 1

// [[[end]]]

/* ************************************************************************** */

extern void pins_init(void);

#endif /* _PINS_H_ */