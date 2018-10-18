#include "shell_commands.h"
#include "calibration.h"
#include "display.h"
#include "events.h"
#include "os/console_io.h"
#include "os/shell/shell.h"
#include "os/shell/shell_command_processor.h"
#include "peripherals/adc.h"
#include "peripherals/nonvolatile_memory.h"
#include "rf_sensor.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/* ************************************************************************** */

// from display.c

int shell_show_bargraphs(int argc, char **argv) {
    if (argc == 3) {
        print("first arg: ");
        print(argv[1]);
        uint16_t forwardWatts = atoi(argv[1]);
        printf(", forwardWatts: %u\r\n", forwardWatts);

        print("second arg: ");
        print(argv[2]);
        double swrValue = atof(argv[2]);
        printf(", swrValue: %u\r\n", swrValue);

        show_power_and_SWR(forwardWatts, swrValue);
    }
    return 0;
}

/* -------------------------------------------------------------------------- */

/*  calibration_packet()

    This function provides data to a calibration routine that runs on an LDG
    Servitor. The Servitor uses this data in conjunction with data from a
    Kenwood TS-480 radio and Alpha 4510 wattmeter to generate frequency
    compensation tables to improve the accuracy of the RF sensor.

    {"forwardADC":"0","forwardWatts":"0.000000","reverseADC":"0","reverseWatts":"0.000000","swr":"0.000000","frequency":"-1"}
*/

// json object represented by null-terminated array of "json field" structs
const json_field_t RF[] = {
    {"forwardADC", &currentRF.forward.value, jsonNumber},
    {"reverseADC", &currentRF.reverse.value, jsonNumber},
    {"matchQuality", &currentRF.matchQuality, jsonNumber},
    {"forwardWatts", &currentRF.forwardWatts, jsonNumber},
    {"reverseWatts", &currentRF.reverseWatts, jsonNumber},
    {"swr", &currentRF.swr, jsonNumber},
    {"frequency", &currentRF.frequency, jsonNumber},
    {NULL, NULL, jsonNumber},
};

int calibration_packet(int argc, char **argv) {
    json_serialize_and_print(&RF[0]);

    return 0;
}

/* -------------------------------------------------------------------------- */

int8_t decode_array_number(char *string) {
    if (!strcmp(string, "forward") || !strcmp(string, "fwd") ||
        !strcmp(string, "f")) {
        return 0;
    }

    if (!strcmp(string, "reverse") || !strcmp(string, "rev") ||
        !strcmp(string, "r")) {
        return 1;
    }

    return atoi(string);
}

int poly(int argc, char **argv) {
    int8_t array = 0;
    int8_t slot = 0;

    switch (argc) {
    case 1:
        println("usage: \tpoly load <array> <slot> <A> <B> <C>");
        println("\tpoly write");
        println("\tpoly read <array> <slot>");
        println("\t<A>, <B>, and <C> are IEEE 754 single precision floats.");
        return 0;
    case 2: // poly write
        if (!strcmp(argv[1], "read")) {
        } else {
            break;
        }

        return 0;
    case 4: // poly read
        if (!strcmp(argv[1], "read")) {
        } else {
            break;
        }

        array = decode_array_number(argv[2]);
        if (array > NUMBER_OF_ARRAYS || array == -1) {
            break;
        }

        slot = atoi(argv[3]);
        if (slot > NUM_OF_BANDS) {
            break;
        }

        polynomial_t tempPoly = calibrationTable[array][slot];

        print_poly(&tempPoly);

        return 0;
    case 7: // poly load
        if (!strcmp(argv[1], "load")) {
        } else {
            break;
        }

        array = decode_array_number(argv[2]);
        if (array > NUMBER_OF_ARRAYS) {
            break;
        }

        slot = atoi(argv[3]);
        if (slot > NUM_OF_BANDS) {
            break;
        }

        calibrationBuffer[array][slot].A = atof(argv[4]);
        calibrationBuffer[array][slot].B = atof(argv[5]);
        calibrationBuffer[array][slot].C = atof(argv[6]);

        return 0;
    default:
        break;
    }
    println("invalid arguments");
    return 0;
}

/* -------------------------------------------------------------------------- */

int fwd(int argc, char **argv) {
    adc_read(0);
    return 0;
}
int rev(int argc, char **argv) {
    adc_read(1);
    return 0;
}

/* -------------------------------------------------------------------------- */

int tune(int argc, char **argv) {
    request_full_tune();
    return 0;
}

/* ************************************************************************** */

void register_all_shell_commands(void) {
    // from display.c
    shell_register_command(shell_show_bargraphs, "bar");

    //
    shell_register_command(calibration_packet, "cal");

    // calibration data
    shell_register_command(poly, "poly");

    shell_register_command(fwd, "fwd");
    shell_register_command(rev, "rev");

    shell_register_command(tune, "tune");
}
