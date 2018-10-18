#include "shell_commands.h"
#include "calibration.h"
#include "display.h"
#include "events.h"
#include "os/console_io.h"
#include "os/log.h"
#include "os/log_macros.h"
#include "os/shell/shell.h"
#include "os/shell/shell_command_processor.h"
#include "peripherals/adc.h"
#include "peripherals/nonvolatile_memory.h"
#include "rf_sensor.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
static uint8_t LOG_LEVEL = L_SILENT;

/* ************************************************************************** */

// Shell command standard return values
#define SHELL_RET_SUCCESS 0
#define SHELL_RET_FAILURE 1

/* ************************************************************************** */

// built-in shell commands

int shell_help(int argc, char **argv) {
    println("-----------------------------------------------");
    shell_print_commands();
    println("-----------------------------------------------");

    return SHELL_RET_SUCCESS;
}

const char argTestUsage[] = "\
This command has no special arguments.\r\n\
It is designed to test the TuneOS shell's arg parsing.\r\n\
\r\n\
Use it like this:\r\n\
\"$ test command arg1 arg2 arg3\"\r\n\
\r\n\
To get this response:\r\n\
Received 4 arguments for test command\r\n\
1 - \"command\" [len:7]\r\n\
2 - \"arg1\" [len:4]\r\n\
3 - \"arg2\" [len:4]\r\n\
4 - \"arg3\" [len:4]\r\n\
";

int shell_arg_test(int argc, char **argv) {
    println("-----------------------------------------------");
    println("SHELL ARG PARSING TEST UTILITY");
    if (argc == 1) {
        print(argTestUsage);
    } else {
        printf("Received %d arguments for test command\r\n", argc - 1);

        // Prints: <argNum> - "<string>" [len:<length>]
        for (uint8_t i = 1; i < argc; i++) {
            printf("%d - \"%s\" [len:%d]\r\n", i, argv[i], strlen(argv[i]));
        }
    }
    println("-----------------------------------------------");

    return SHELL_RET_SUCCESS;
}

/* -------------------------------------------------------------------------- */
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

int calibration_packet(int argc, char **argv) {
    print("{");
    printf("\"forwardADC\":%f,", currentRF.forward.value);
    printf("\"reverseADC\":%f,", currentRF.reverse.value);
    printf("\"matchQuality\":%f,", currentRF.matchQuality);
    printf("\"forwardWatts\":%f,", currentRF.forwardWatts);
    printf("\"reverseWatts\":%f,", currentRF.reverseWatts);
    printf("\"swr\":%f,", currentRF.swr);
    printf("\"frequency\":%d", currentRF.frequency);
    println("}");
}

/* -------------------------------------------------------------------------- */

int edit_log_levels(int argc, char **argv) {
    switch (argc) {
    case 1:
        println("log set <file> <value>");
        println("log read");
        return 0;
    case 2:
        if (strcmp(argv[1], "read") == 0) {
            print_log_list();
        }
        return 0;
    case 4:
        if (strcmp(argv[1], "set") == 0) {
            uint8_t file = atoi(argv[2]);
            uint8_t i = 0;
            for (i = 0; i < logDatabase.numberOfFiles; i++) {
                if (!strcmp(argv[3], level_names[i])) {
                    log_level_edit(file, i);
                    return 0;
                }
            }
        }
    default:
        println("invalid arguments");
        return 0;
    }
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

/* ************************************************************************** */

int fwd(int argc, char **argv) { adc_read(0); }
int rev(int argc, char **argv) { adc_read(1); }

/* ************************************************************************** */

int tune(int argc, char **argv) { request_full_tune(); }

/* ************************************************************************** */

void register_all_shell_commands(void) {
    log_register();

    // built-in shell commands
    shell_register_command(shell_help, "help");
    shell_register_command(shell_arg_test, "test");

    // from display.c
    shell_register_command(shell_show_bargraphs, "bar");

    //
    shell_register_command(calibration_packet, "cal");

    // log level controls
    shell_register_command(edit_log_levels, "log");

    // calibration data
    shell_register_command(poly, "poly");

    shell_register_command(fwd, "fwd");
    shell_register_command(rev, "rev");

    shell_register_command(tune, "tune");
}
