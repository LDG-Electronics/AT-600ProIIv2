#include "adc.h"
#include "pic18f46k42.h"

/* ************************************************************************** */

void adc_init(void) {
    // ADC reference stuff
    FVRCONbits.ADFVR = 0b11; // ADC voltage reference is 4.096 volts
    FVRCONbits.EN = 1;       // Enable Fixed Voltage Reference

    ADREFbits.NREF = 0;    // Negative Voltage Reference, set to Vss
    ADREFbits.PREF = 0b11; // Positive Voltage Reference, set to FVR

    ADCON0bits.FM = 1; // adc result is right-justified

    ADCON0bits.CS = 0; // FOSC, divided by ADCLK
    ADCLK = 0b011111;  // FOSC/64

    ADCON0bits.ON = 1; // Enable ADC peripheral
}

uint16_t adc_measure(uint8_t channel) {
    // CHS0 only selects between AN1/AN0
    ADPCH = channel;

    // Engage
    ADCON0bits.GO = 1;
    while (ADCON0bits.GO) {
        // Wait for the conversion to finish
    }

    return ADRES;
}