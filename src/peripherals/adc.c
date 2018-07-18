#include "../includes.h"

/* ************************************************************************** */

void adc_init(void) {
    FVRCONbits.ADFVR = 0b11; // ADC voltage reference is 4.096 volts
    FVRCONbits.EN = 1;       // Enable Fixed Voltage Reference

    ADREFbits.NREF = 0;    // Negative Voltage Reference, set to Vss
    ADREFbits.PREF = 0b11; // Positive Voltage Reference, set to FVR

    ADCON0bits.FM = 1; // adc result is right-justified
    ADCON0bits.CS = 1; // FRC Clock

    ADCON0bits.ON = 1; // Enable ADC peripheral
}

uint16_t adc_measure(uint8_t channel) {
    // CHS0 only selects between AN1/AN0
    ADPCH = channel;

    // Engage
    ADCON0bits.GO = 1;
    while (ADCON0bits.GO == 1) {
        // Wait for the conversion to finish
    }

    return ADRES;
}