#ifndef _ADC_H_
#define _ADC_H_

/* ************************************************************************** */

/*  Not a lot to say about this driver....
    
    Vdd and Vss are the reference voltages, acquisition time is set to as
    slow as possible, the result is right justified.

    Eventually, adc_init could allow for reconfiguration at runtime, but
    currently there's nothing worth changing.

    adc_measure() initiates a/d conversion on the selected channel, waits until
    the conversion finishes, then returns the result.
*/

// Setup and configuration
void adc_init(void);

// Actually measuring shit
uint16_t adc_measure(uint8_t channel);

#endif