#ifndef _ADC_H_
#define _ADC_H_

#include <stdint.h>

/* ************************************************************************** */

typedef struct {
    float value; // in millivolts
    uint16_t discardedSamples;
} adc_result_t;

/* ************************************************************************** */

// Setup and configuration
extern void adc_init(void);

// Actually measuring shit
extern uint16_t adc_single_sample(uint8_t channel);

//
extern adc_result_t adc_read(uint8_t channel);

#endif