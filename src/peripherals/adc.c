#include "adc.h"
#include "../os/log_macros.h"
#include "pic18f46k42.h"
static uint8_t LOG_LEVEL = L_TRACE;

/* ************************************************************************** */



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

    log_register();
}

/* ************************************************************************** */

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

/* -------------------------------------------------------------------------- */

#define NUMBER_OF_SAMPLES 16
uint16_t samplebuffer[NUMBER_OF_SAMPLES];

void clear_sample_buffer(void) {
    for (uint8_t i = 0; i < NUMBER_OF_SAMPLES; i++) {
        samplebuffer[i] = 0;
    }
}

void print_sample_buffer(void) {
    // print them out?
    LOG_INFO({
        println("");
        for (uint8_t i = 0; i < NUMBER_OF_SAMPLES; i++) {
            printf("(%d, %d)\r\n", i, samplebuffer[i]);
        }
    });
}

/* -------------------------------------------------------------------------- */

adc_result_t adc_take_average(uint8_t channel) {
    clear_sample_buffer();
    uint16_t totalSamples = 0;
    uint8_t samples = 0;

    ADPCH = channel;

    while (samples < NUMBER_OF_SAMPLES) {
        ADCON0bits.GO = 1;
        while (ADCON0bits.GO) {
            // Wait for the conversion to finish
        }

        if (ADRES != 0) {
            samplebuffer[samples++] = ADRES;
        }

        totalSamples++;
        if (totalSamples == 255) {
            break;
        }
    }

    double sum = 0;
    for (uint8_t i = 0; i < NUMBER_OF_SAMPLES; i++) {
        sum += samplebuffer[i];
    }

    adc_result_t measurement;
    measurement.value = sum / NUMBER_OF_SAMPLES;
    measurement.discardedSamples = totalSamples - NUMBER_OF_SAMPLES;

    return measurement;
}