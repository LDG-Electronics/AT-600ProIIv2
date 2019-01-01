#include "../calibration.h"
#include "../display.h"
#include "../os/serial_port.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

/* ************************************************************************** */

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

void shell_poly(int argc, char **argv) {
    int8_t array = 0;
    int8_t slot = 0;

    switch (argc) {
    case 1: // usage
        print("usage: ");
        println("\tpoly write");
        println("\tpoly read <fwd|rev> <band>");
        println("\tpoly read all");
        println("\tpoly load <fwd|rev> <band> <A> <B> <C>");
        println("\t<A>, <B>, and <C> are IEEE 754 single precision floats.");
        return;
    case 2: // poly write
        if (!strcmp(argv[1], "write")) {
        } else {
            break;
        }

        return;
    case 3: // poly read all
        if (!strcmp(argv[1], "read")) {
        } else {
            break;
        }

        if (!strcmp(argv[2], "all")) {
        } else {
            break;
        }

        println("forwardCalibrationTable:");
        for (uint8_t band = 0; band < NUM_OF_BANDS; band++) {
            print_poly(forwardCalibrationTable[band]);
            println("");
        }

        println("reverseCalibrationTable:");
        for (uint8_t band = 0; band < NUM_OF_BANDS; band++) {
            print_poly(reverseCalibrationTable[band]);
            println("");
        }

        return;
    case 4: // poly read <fwd|rev> <band>
        if (!strcmp(argv[1], "read")) {
        } else {
            break;
        }

        polynomial_t *poly;

        if (!strcmp(argv[2], "fwd")) {
            poly = &forwardCalibrationTable;
        } else if (!strcmp(argv[2], "rev")) {
            poly = &reverseCalibrationTable;
        } else {
            break;
        }

        uint8_t band = atoi(argv[3]);
        if (band > NUM_OF_BANDS) {
            break;
        }

        print_poly(poly[band]);

        return;
    case 7: // poly load <fwd|rev> <band> <A> <B> <C>
        if (!strcmp(argv[1], "load")) {
        } else {
            break;
        }

        polynomial_t *poly;

        if (!strcmp(argv[2], "fwd")) {
            poly = &forwardCalibrationTable;
        } else if (!strcmp(argv[2], "rev")) {
            poly = &reverseCalibrationTable;
        } else {
            break;
        }

        uint8_t band = atoi(argv[3]);
        if (band > NUM_OF_BANDS) {
            break;
        }

        polynomial_t temp;
        temp.A = atof(argv[4]);
        temp.B = atof(argv[5]);
        temp.C = atof(argv[6]);

        poly[band] = temp;

        return;
    default:
        break;
    }
    println("invalid arguments");
    return;
}