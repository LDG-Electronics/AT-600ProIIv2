#include "os/serial_port.h"
#include "peripherals/nonvolatile_memory.h"
#include "shell_command_processor.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

/* ************************************************************************** */

void sh_eeprom(int argc, char **argv) {
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