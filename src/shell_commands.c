#include "shell_commands.h"
#include "calibration.h"
#include "display.h"
#include "events.h"
#include "os/serial_port.h"
#include "os/shell/shell.h"
#include "os/shell/shell_command_processor.h"
#include "os/shell/shell_json.h"
#include "peripherals/adc.h"
#include "peripherals/nonvolatile_memory.h"
#include "rf_sensor.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/* ************************************************************************** */

// from display.c

void shell_show_bargraphs(int argc, char **argv) {
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
    return;
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
    {"forwardADC", &(currentRF.forward.value), jsonDouble},
    {"reverseADC", &(currentRF.reverse.value), jsonDouble},
    {"matchQuality", &(currentRF.matchQuality), jsonDouble},
    {"forwardWatts", &(currentRF.forwardWatts), jsonDouble},
    {"reverseWatts", &(currentRF.reverseWatts), jsonDouble},
    {"swr", &(currentRF.swr), jsonDouble},
    {"frequency", &(currentRF.frequency), jsonU16},
    {NULL, NULL, jsonObject},
};

void calibration_packet(int argc, char **argv) {
    json_serialize_and_print(&RF[0]);
}

/* -------------------------------------------------------------------------- */

static int8_t decode_array_number(char *string) {
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

void poly(int argc, char **argv) {
    int8_t array = 0;
    int8_t slot = 0;

    switch (argc) {
    case 1:
        println("usage: \tpoly load <array> <slot> <A> <B> <C>");
        println("\tpoly write");
        println("\tpoly read <array> <slot>");
        println("\t<A>, <B>, and <C> are IEEE 754 single precision floats.");
        return;
    case 2: // poly write
        if (!strcmp(argv[1], "read")) {
        } else {
            break;
        }

        return;
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

        return;
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

        return;
    default:
        break;
    }
    println("invalid arguments");
    return;
}

/* -------------------------------------------------------------------------- */

void fwd(int argc, char **argv) { adc_read(0); }
void rev(int argc, char **argv) { adc_read(1); }

/* -------------------------------------------------------------------------- */

void tune(int argc, char **argv) { request_full_tune(); }

/* -------------------------------------------------------------------------- */
// Diagnostic shell commands for nonvolatile_memory driver

void shell_eeprom(int argc, char **argv) {
    switch (argc) {
    case 1:
        println("usage: \teeprom write <address> <data>");
        println("\teeprom read <address>");
        return;

    case 3:
        if (!strcmp(argv[1], "read")) {
            uint16_t address = atoi(argv[2]);

            printf("%02x\r\n", internal_eeprom_read(address));
            return;
        }
        break;

    case 4:
        if (!strcmp(argv[1], "write")) {
            uint16_t address = atoi(argv[2]);
            uint8_t data = atoi(argv[3]);

            internal_eeprom_write(address, data);
            printf("%02x\r\n", internal_eeprom_read(address));
            return;
        }
        break;

    default:
        break;
    }
    println("invalid arguments");
    return;
}

/* -------------------------------------------------------------------------- */

static NVM_address_t decode_address(char *string) {
    uint8_t length = strlen(string);

    bool decimal = true;

    for (uint8_t i = 0; i < length; i++) {
        if (!isdigit(string[i])) {
            decimal = false;
        }
    }
    if (decimal) {
        int32_t temp = atol(string);
        if ((temp >= 0) && (temp < FLASH_SIZE)) {
            return (NVM_address_t)temp;
        }
    }
    return 0xffffff;
}

static int16_t decode_data(char *string) {
    uint8_t length = strlen(string);

    bool decimal = true;
    bool hex = true;

    for (uint8_t i = 0; i < length; i++) {
        if (!isdigit(string[i])) {
            decimal = false;
        }
    }
    if (decimal) {
        return atoi(string);
    }

    for (uint8_t i = 0; i < length; i++) {
        if (!isxdigit(string[i])) {
            hex = false;
        }
    }
    if (hex) {
        return xtoi(string);
    }
    return -1;
}

void shell_flash(int argc, char **argv) {
    switch (argc) {
    case 1:
        println("usage: \tflash write <address> <data>");
        println("\tflash read <address>");
        println("\tflash bread <address>");
        println("\tValid addresses are between 0 and 65355:");
        println("\tData must be a single byte in decimal or hex format");
        return;

    case 3:
        if (!strcmp(argv[1], "read")) {

            // parse address
            NVM_address_t address = decode_address(argv[2]);
            if (address == 0xffffff) {
                println("invalid address");
                return;
            }

            printf("%02x\r\n", flash_read_byte(address));
            return;
        } else if (!strcmp(argv[1], "bread")) {

            // parse address
            NVM_address_t address = decode_address(argv[2]);
            if (address == 0xffffff) {
                println("invalid address");
                return;
            }

            // Read existing block into buffer
            uint8_t buffer[FLASH_BUFFER_SIZE];
            flash_read_block(address, buffer);
            print_flash_buffer(address, buffer);
            return;
        }
        break;

    case 4:
        if (!strcmp(argv[1], "write")) {
            // parse address
            NVM_address_t address = decode_address(argv[2]);
            if (address == 0xffffff) {
                println("invalid address");
                return;
            }

            // parse data
            int16_t test = decode_data(argv[3]);
            if (test < 0 || test > 255) {
                println("invalid data");
                return;
            }
            uint8_t newData = (uint8_t)test;

            // return if the address already contains the data we want
            uint8_t existingData = flash_read_byte(address);
            if (existingData == newData) {
                return;
            }

            // compare the two datas bit-by-bit, checking for 0->1 transitions
            bool mustErase = false;
            for (uint8_t i = 0; i < 8; i++) {
                if (!(existingData & (1 << i)) && (newData & (1 << i))) {
                    mustErase = true;
                }
            }

            // Read existing block into buffer
            uint8_t buffer[FLASH_BUFFER_SIZE];
            flash_read_block(address, buffer);

            // write newData into the buffer
            buffer[address & FLASH_ELEMENT_MASK] = newData;

            // only erase if we need to
            if (mustErase) {
                flash_block_erase(address);
            }

            // Write the modified buffer back into flash
            flash_block_write(address, buffer);

            // Verify that the write worked
            flash_read_block(address, buffer);

            return;
        }
        break;

    default:
        break;
    }
    println("invalid arguments");
}