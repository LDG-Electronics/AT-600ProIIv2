#include "sh_adc.h"
#include "../os/serial_port.h"
#include "../os/stopwatch.h"
#include "../peripherals/adc.h"
#include "../peripherals/pic_header.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

/* ************************************************************************** */

void sh_adc_init(void) {
    ADREFbits.NREF = 0;    // Negative Voltage Reference, set to Vss
    ADREFbits.PREF = 0b11; // Positive Voltage Reference, set to FVR

    ADCON0bits.FM = 1; // adc result is right-justified

    ADCON0bits.CS = 0; // FOSC, divided by ADCLK
    ADCLK = 0b011111;  // FOSC/64

    ADCON0bits.ON = 1; // Enable ADC peripheral
}

/* -------------------------------------------------------------------------- */
/*  controlling the adc from the shell/python

    I need to test adc settings faster than is practical by editing, building,
    and uploading. We're going to expose ADC registers to the shell and do this
    like a real programmer.

    ADCLK - clock selection, <0...64> (FOSC/n)
    ADPRE - precharge time, <0...8191>
    ADACQ - acquisition time, <0...8191>
    ADCAP - additional capacitance, <0...31> (n pF)

    I also need to experiment with sampling techniques and DSP. We're going to
    do tests like grabbing 256 adc samples as rapidly as possible, and then
    piping them to the shell and having the servitor draw pretty graphs.

*/
#define ADCLK_MAX 63
void sh_adc_set_clock(int16_t value) {
    if (value >= 0 && value <= ADCLK_MAX) {
        ADCLK = value;
        // printf("ADCLK: %u\r\n", ADCLK);
    }
}

#define ADPRE_MAX 8191
void sh_adc_set_precharge_time(int16_t value) {
    if (value >= 0 && value <= ADPRE_MAX) {
        ADPRE = value;
        // printf("ADPRE: %u\r\n", ADPRE);
    }
}

#define ADACQ_MAX 8191
void sh_adc_set_acquisition_time(int16_t value) {
    if (value >= 0 && value <= ADACQ_MAX) {
        ADACQ = value;
        // printf("ADACQ: %u\r\n", ADACQ);
    }
}

#define ADCAP_MAX 31
void sh_adc_set_additional_capacitance(int16_t value) {
    if (value >= 0 && value <= ADCAP_MAX) {
        ADCAP = value;
        // printf("ADCAP: %u\r\n", ADCAP);
    }
}

void sh_print_settings(void) {
    printf("ADCLK: %u\r\n", ADCLK);
    printf("ADPRE: %u\r\n", ADPRE);
    printf("ADACQ: %u\r\n", ADACQ);
    printf("ADCAP: %u\r\n", ADCAP);
}

/* -------------------------------------------------------------------------- */

#define NUM_OF_TEST_SAMPLES 64
uint16_t fwdArray[NUM_OF_TEST_SAMPLES];
uint16_t revArray[NUM_OF_TEST_SAMPLES];

void print_array(const char *label, uint16_t *array) {
    print("\"");
    print(label);
    print("\"");

    println("[");
    print("  ");
    for (uint16_t i = 0; i < NUM_OF_TEST_SAMPLES; i++) {
        printf("%4u", array[i]);
        if (i < NUM_OF_TEST_SAMPLES - 1) {
            print(",");

            if (((i + 1) % 8) == 0) {
                println("");
                if (i < NUM_OF_TEST_SAMPLES - 1) {
                    print("  ");
                }
            }
        }
    }
    println("");
    print("]");
}

void sh_adc_read(void) {
    // Collect measurements
    for (uint16_t i = 0; i < NUM_OF_TEST_SAMPLES; i++) {
        fwdArray[i] = adc_read(0);
        revArray[i] = adc_read(1);
    }

    println("{");
    print_array("forward", &fwdArray);
    println(",");
    print_array("reverse", &revArray);
    println("}");
}

/* -------------------------------------------------------------------------- */

void sh_adc(int argc, char **argv) {
    uint16_t value = 0;
    switch (argc) {
    case 1: // usage
        print("usage: ");
        println("\tadc read");
        println("\tadc settings");
        println("\tadc set <setting> <value>");
        return;
    case 2:
        if (!strcmp(argv[1], "read")) { // adc read
            sh_adc_read();
            return;

        } else if (!strcmp(argv[1], "settings")) { // adc settings
            sh_print_settings();
            return;
        }
        break;
    case 4: // adc set <setting> <value>
        if (!strcmp(argv[1], "set")) {
            if (!strcmp(argv[2], "ADCLK")) { // adc set ADCLK <value>
                sh_adc_set_clock(atoi(argv[3]));
                return;
            } else if (!strcmp(argv[2], "ADPRE")) { // adc set ADPRE <value>
                sh_adc_set_precharge_time(atoi(argv[3]));
                return;
            } else if (!strcmp(argv[2], "ADACQ")) { // adc set ADACQ <value>
                sh_adc_set_acquisition_time(atoi(argv[3]));
                return;
            } else if (!strcmp(argv[2], "ADCAP")) { // adc set ADCAP <value>
                sh_adc_set_additional_capacitance(atoi(argv[3]));
                return;
            }
        }
        break;
    }
    println("invalid arguments");
}