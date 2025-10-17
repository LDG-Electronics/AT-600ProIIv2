#include "nvm_table.h"
#include "os/serial_port.h"
#include "shell_command_processor.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/* ************************************************************************** */

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

void sh_memory(int argc, char **argv) {
    switch (argc) {
    case 1:
        println("usage: \tmemory write <slot> <data>");
        println("\tmemory read <slot>");
        return;
    case 3:
        if (!strcmp(argv[1], "read")) {
            // parse address
            uint16_t slot = atoi(argv[2]);

            // read the memory at that address
            print_table_entry(nvm_table_read(slot));
            println("");

            return;
        }
        break;
    case 4:
        if (!strcmp(argv[1], "write")) {
            // parse address
            uint16_t slot = atoi(argv[2]);

            // parse data
            int16_t data = decode_data(argv[3]);
            if (data < 0 || data > 255) {
                println("invalid data");
                return;
            }

            // create a new table_entry_t object and load it with the user input
            table_entry_t entry = new_table_entry();
            entry.contents[0] = data;
            entry.contents[1] = data;

            nvm_table_write(slot, entry);

            return;
        }
        break;

    default:
        break;
    }
    println("invalid arguments");
}