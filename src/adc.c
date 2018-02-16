#include "includes.h"

/* ************************************************************************** */

void adc_init(void)
{
    ADCON0 = 0;
    
    ADCLK = 0xff;

    ADREFbits.NREF = 0; // Negative Voltage Reference, set to Vss
    ADREFbits.PREF = 0b00; // Positive Voltage Reference, set to Vdd
    
    ADCON0bits.FM = 1; // adc result is right-justified
    
    ADCON0bits.ON = 1; // Enable ADC peripheral
}

uint16_t adc_measure(uint8_t channel)
{
    // CHS0 only selects between AN1/AN0
    ADPCH = channel;

    // Engage
    ADCON0bits.GO = 1;
    // Wait for the conversion to finish
    while (ADCON0bits.GO == 1); 
    
    return ((ADRESH << 8) | ADRESL);
}