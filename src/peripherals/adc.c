#include "adc.h"
#include "../os/log_macros.h"
#include "fixed_voltage_reference.h"
#include "pic18f47k42.h"
static uint8_t LOG_LEVEL = L_SILENT;

/* ************************************************************************** */

void adc_init(void) {
    fvr_enable();

    ADREFbits.NREF = 0;    // Negative Voltage Reference, set to Vss
    ADREFbits.PREF = 0b11; // Positive Voltage Reference, set to FVR

    ADCON0bits.FM = 1; // adc result is right-justified

    ADCON0bits.CS = 0; // FOSC, divided by ADCLK
    ADCLK = 0b011111;  // FOSC/64

    ADCON0bits.ON = 1; // Enable ADC peripheral

    log_register();
}

/* -------------------------------------------------------------------------- */

#define NUM_OF_CHANNELS 2
#define _1024mV FVR_GAIN_1X
#define _2048mV FVR_GAIN_2X
#define _4096mV FVR_GAIN_4X

#define adc_select_channel(channel) ADPCH = channel

//! Warning - input not sanitized
static void adc_select_scale(uint8_t scale) {
    fvr_set_adc_buffer_gain(scale);

    // wait for FVR to stabilize
    fvr_wait_until_ready();
}

uint16_t adc_convert(uint8_t channel, uint8_t scale) {
    adc_select_channel(channel);
    adc_select_scale(scale);

    // Engage
    ADCON0bits.GO = 1;
    while (ADCON0bits.GO) {
        // Wait for the conversion to finish
    }

    return ADRES;
}

/* -------------------------------------------------------------------------- */

uint16_t adc_read(uint8_t channel) {
    LOG_TRACE({ println("adc_read"); });

    static uint8_t maxVoltage[NUM_OF_CHANNELS] = {_4096mV, _1024mV};

    uint16_t measurement = 0;

    while (1) {
        adc_select_scale(maxVoltage[channel]);
        measurement = adc_convert(channel, maxVoltage[channel]);

        LOG_DEBUG({
            printf("channel: %d, maxVoltage: %d, measurement: %u\r\n", channel,
                   maxVoltage[channel], measurement);
        });

        if (measurement < 1750) {
            if (maxVoltage[channel] > _1024mV) {
                LOG_INFO({ println("decreasing maxVoltage"); });
                maxVoltage[channel]--;
            } else {
                break;
            }
        } else if (measurement > 3900) {
            if (maxVoltage[channel] < _4096mV) {
                LOG_INFO({ println("increasing maxVoltage"); });
                maxVoltage[channel]++;
            } else {
                break;
            }
        } else {
            break;
        }
    }

    // Convert measurement to millivolts as needed.
    if (maxVoltage[channel] == _1024mV) {
        measurement /= 4;
    } else if (maxVoltage[channel] == _2048mV) {
        measurement /= 2;
    }

    return measurement;
}