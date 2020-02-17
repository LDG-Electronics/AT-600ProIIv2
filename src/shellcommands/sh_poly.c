#include "calibration.h"
#include "os/serial_port.h"
#include "os/shell/shell_command_processor.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

/* ************************************************************************** */

void sh_poly(int argc, char **argv) {
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
            // TODO: commit uploaded polys to ROM?
            // this requires additional work in calibration.c
            return;
        }
        break;
    case 3: // poly read all
        if ((!strcmp(argv[1], "read")) && (!strcmp(argv[2], "all"))) {
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
        }
        break;
    case 4: // poly read <fwd|rev> <band>
        if (!strcmp(argv[1], "read")) {
            // which band?
            uint8_t band = atoi(argv[3]);
            if (band >= NUM_OF_BANDS) {
                break;
            }

            // which poly table?
            if (!strcmp(argv[2], "fwd")) {
                print_poly(forwardCalibrationTable[band]);
            } else if (!strcmp(argv[2], "rev")) {
                print_poly(reverseCalibrationTable[band]);
            } else {
                break;
            }

            return;
        }
        break;
    case 7: // poly load <fwd|rev> <band> <A> <B> <C>
        if (!strcmp(argv[1], "load")) {
            // which band?
            uint8_t band = atoi(argv[3]);
            if (band >= NUM_OF_BANDS) {
                break;
            }

            // decode poly
            polynomial_t poly;
            poly.A = atof(argv[4]);
            poly.B = atof(argv[5]);
            poly.C = atof(argv[6]);

            // which poly table?
            if (!strcmp(argv[2], "fwd")) {
                forwardCalibrationTable[band] = poly;
            } else if (!strcmp(argv[2], "rev")) {
                reverseCalibrationTable[band] = poly;
            } else {
                break;
            }

            return;
        }
        break;
    default:
        break;
    }
    println("invalid arguments");
    return;
}

REGISTER_SHELL_COMMAND(sh_poly, "poly");