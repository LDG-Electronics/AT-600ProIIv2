#ifndef _ADC_H_
#define _ADC_H_

#include <stdint.h>

/* ************************************************************************** */

typedef struct {
    double value;
    uint16_t discardedSamples;
} adc_result_t;

/* ************************************************************************** */

/*  Not a lot to say about this driver....
    
    Vdd and Vss are the reference voltages, acquisition time is set to as
    slow as possible, the result is right justified.

    Eventually, adc_init could allow for reconfiguration at runtime, but
    currently there's nothing worth changing.

    adc_measure() initiates a/d conversion on the selected channel, waits until
    the conversion finishes, then returns the result.
*/

extern void print_sample_buffer(void);

// Setup and configuration
extern void adc_init(void);

// Actually measuring shit
extern uint16_t adc_measure(uint8_t channel);

// 
extern adc_result_t adc_take_average(uint8_t channel);

#endif