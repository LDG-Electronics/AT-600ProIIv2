#ifndef _PINS_H_
#define _PINS_H_

#include "peripherals/pps.h"
#include <stdbool.h>

/* ************************************************************************** */
/* [[[cog
    from codegen import pins, fmt
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
extern bool read_TUNE_BUTTON_PIN(void);
extern bool read_ANT_BUTTON_PIN(void);

// GPIO write functions
extern void set_POWER_LED_PIN(bool value);
extern void set_FP_CLOCK_PIN(bool value);
extern void set_RADIO_CMD_PIN(bool value);
extern void set_ANT_LED_PIN(bool value);
extern void set_RELAY_CLOCK_PIN(bool value);
extern void set_RELAY_DATA_PIN(bool value);
extern void set_RELAY_STROBE_PIN(bool value);
extern void set_BYPASS_LED_PIN(bool value);
extern void set_FP_STROBE_PIN(bool value);
extern void set_FP_DATA_PIN(bool value);

// PPS initialization macros
#define PPS_USB_TX_PIN PPS_OUTPUT(C, 6)
#define PPS_USB_RX_PIN PPS_INPUT(C, 7)
#define PPS_DEBUG_TX_PIN PPS_OUTPUT(D, 2)
#define PPS_DEBUG_RX_PIN PPS_INPUT(D, 3)
#define PPS_FREQ_PIN PPS_INPUT(E, 0)

// ADC Channel Select macros
#define ADC_FWD_PIN 0
#define ADC_REV_PIN 1

// [[[end]]]

/* ************************************************************************** */

extern void pins_init(void);

#endif /* _PINS_H_ */