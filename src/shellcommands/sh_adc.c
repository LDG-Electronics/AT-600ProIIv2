#include "sh_adc.h"
#include "../os/serial_port.h"
#include "../os/stopwatch.h"
#include "../peripherals/adc.h"
#include "../peripherals/pic_header.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

/* ************************************************************************** */

// uint8_t array1[256]; // <-- compiles
// uint8_t array2[257]; // <-- compiles
// uint8_t array3[512]; // <-- compiles
// uint8_t array4[1024]; // <-- compiles

// void some_func(void) {
//     uint8_t array1[256]; // <-- compiles
//     uint8_t array2[257]; // <-- DOES NOT COMPILE
//     uint8_t array3[512]; // <-- DOES NOT COMPILE
//     uint8_t array4[1024]; // <-- DOES NOT COMPILE
// }

/* ************************************************************************** */
/*  controlling the adc from the shell/python

    I need to test adc settings faster than is practical by editing, building,
    and uploading. We're going to expose ADC registers to the shell and do this
    like a real programmer.

    ADCLK - clock selection, <0...64> (FOSC/n)
    ADPRE - precharge time, <0...8191>
    ADACQ - acquisition time, <0...8191>
    ADCAP - additional capacitance, <0...31> (n pF)

    I also need to expiriment with sampling techniques and DSP. We're going to
    do tests like grabbing 256 adc samples as rapidly as possible, and then
    piping them to the shell and having the servitor draw pretty graphs.

*/
void sh_adc_select_clock(uint8_t value) { ADCLK = value; }
void sh_adc_set_precharge_time(uint16_t value) { ADPRE = value; }
void sh_adc_set_acquisition_time(uint16_t value) { ADACQ = value; }
void sh_adc_set_additional_capacitance(uint8_t value) { ADCAP = value; }

void sh_adc_init(void) {
    ADREFbits.NREF = 0;    // Negative Voltage Reference, set to Vss
    ADREFbits.PREF = 0b11; // Positive Voltage Reference, set to FVR

    ADCON0bits.FM = 1; // adc result is right-justified

    ADCON0bits.CS = 0; // FOSC, divided by ADCLK
    ADCLK = 0b011111;  // FOSC/64

    ADCON0bits.ON = 1; // Enable ADC peripheral
}

#define NUM_OF_TEST_SAMPLES 500
uint16_t fwdArray[NUM_OF_TEST_SAMPLES];
uint16_t revArray[NUM_OF_TEST_SAMPLES];

void sh_adc_read(void) {

    us_stopwatch_begin();
    // Collect measurements
    for (uint16_t i = 0; i < NUM_OF_TEST_SAMPLES; i++) {
        fwdArray[i] = adc_read(0);
        revArray[i] = adc_read(1);
    }
    us_stopwatch_println();

    println("{");
    print("  \"forward\":");
    println("[");
    print("    ");
    for (uint16_t i = 0; i < NUM_OF_TEST_SAMPLES; i++) {
        printf("%4u", fwdArray[i]);
        if (i < NUM_OF_TEST_SAMPLES - 1) {
            print(",");

            if (((i + 1) % 16) == 0) {
                println("");
                if (i < NUM_OF_TEST_SAMPLES - 1) {
                    print("    ");
                }
            }
        }
    }
    println("");
    println("  ],");

    print("  \"reverse\":");
    println("[");
    print("    ");
    for (uint16_t i = 0; i < NUM_OF_TEST_SAMPLES; i++) {
        printf("%4u", revArray[i]);
        if (i < NUM_OF_TEST_SAMPLES - 1) {
            print(",");

            if (((i + 1) % 16) == 0) {
                println("");
                if (i < NUM_OF_TEST_SAMPLES - 1) {
                    print("    ");
                }
            }
        }
    }
    println("");
    println("  ]");
    println("}");
}

void adc(int argc, char **argv) {
    switch (argc) {
    case 1: // usage
        print("usage: ");
        println("\tadc read");
        println("\tadc");
        println("\tadc");
        println("\tadc");
        println("\tadc");
        return;
    case 2: // adc read
        if (!strcmp(argv[1], "read")) {
            sh_adc_read();
            return;
        }
        break;
    }
    println("invalid arguments");
}