#include "os/serial_port.h"
#include "peripherals/nonvolatile_memory.h"
#include "shell_command_processor.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/* ************************************************************************** */

static NVM_address_t decode_address(char *string) {
    bool decimal = true;

    uint8_t i = 0;
    while (string[i]) {
        if (!isdigit(string[i++])) {
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
    bool decimal = true;
    bool hex = true;

    uint8_t i = 0;
    while (string[i]) {
        if (!isdigit(string[i++])) {
            decimal = false;
        }
    }
    if (decimal) {
        return atoi(string);
    }

    i = 0;
    while (string[i]) {
        if (!isxdigit(string[i++])) {
            hex = false;
        }
    }
    if (hex) {
        return strtol(string, NULL, 16);
    }
    return -1;
}

/* -------------------------------------------------------------------------- */

void sh_flash(int argc, char **argv) {
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

            print_flash_block(address);

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
            int16_t data = decode_data(argv[3]);
            if (data < 0 || data > 255) {
                println("invalid data");
                return;
            }
            flash_write_byte(address, data);

            return;
        }
        break;

    default:
        break;
    }
    println("invalid arguments");
}