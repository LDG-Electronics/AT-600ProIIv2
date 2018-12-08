#include "sh_nonvolatile_memory.h"
#include "../os/serial_port.h"
#include "../peripherals/nonvolatile_memory.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

/* ************************************************************************** */

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