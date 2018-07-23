#include "shell_commands.h"
#include "calibration.h"
#include "display.h"
#include "os/console_io.h"
#include "os/log.h"
#include "os/log_macros.h"
#include "os/shell/shell.h"
#include "os/shell/shell_command_processor.h"
#include "peripherals/nonvolatile_memory.h"
#include "rf_sensor.h"
#include <stdlib.h>
#include <string.h>
static uint8_t LOG_LEVEL = L_TRACE;

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
/*  Parameter commands

    get and set can be used to read and write the vast majority of system state
    variables.

    get is pretty self explanatory: it returns the current setting of the
    requested parameter.

    TODO: revise this paragraph to emphasize the last line and sets role in it
    set is slightly trickier: Many variables require some action to be performed
    when they're modified. This command has a duty to update system variables
    responsibly, whatever that takes. It's unacceptable for the system to
    rendered inoperable or otherwise damaged by modifying a parameter via set.

    Parameter list:
    currentRelays <- from relays.c
        capacitors
        inductors
        z relay
    currentRF <- from RF_sensor.c
        forward
        forwardWatts
        reverse
        reveresWatts
        SWR
        frequency
    systemFlags <- from flags.c
        ant1Bypass
        ant2Bypass
        antenna
        autoMode
        peakMode
        scaleMode
        powerStatus
    currentTime
    tuning status
    current display

    memories
        recall memories by frequency
        recall memories by date

    system information
        product name
        serial number
        sw version
        board revision
        compilation date
        service history:

    metrics
        lifetime tune count
        lifetime relay write count
        highest power level recorded

*/

const char getParamUsage[] = "\
This command is used to read the current value of settings from the tuner.\r\n\
Available parameters are:\r\n\
*** INSERT PARAMETER LIST HERE ***\r\n\
";

int shell_get_param(int argc, char **argv) {
    if (argc == 1) {
        print(getParamUsage);
        return;
    }
    if (!strcmp(argv[1], "")) {
    }
}

const char setParamUsage[] = "\
This command is used to modify the current value of settings from the tuner.\r\n\
Available parameters are:\r\n\
*** INSERT PARAMETER LIST HERE ***\r\n\
";

int shell_set_param(int argc, char **argv) {
    if (argc == 1) {
        print(setParamUsage);
        return;
    }
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

/*  print_RF_data_packet()

    This function provides data to a calibration routine that runs on an LDG
    Servitor. The Servitor uses this data in conjunction with data from a
    Kenwood TS-480 radio and Alpha 4510 wattmeter to generate frequency
    compensation tables to improve the accuracy of the RF sensor.

    (F Fw R Rw SWR      frequency)
    (0 0  0 0  0.000000 0)
*/

void print_RF_data_packet(void) {
    print("(");
    printf("%d ", currentRF.forward);
    printf("%f ", currentRF.forwardWatts);
    printf("%d ", currentRF.reverse);
    printf("%f ", currentRF.reverseWatts);
    printf("%f ", currentRF.swr);
    printf("%d", currentRF.frequency);
    println(")");
}

int calibration_packet(int argc, char **argv) {
    // if called with no argument, print the entire RF data packet
    if (argc == 1) {
        print_RF_data_packet();
        return 0;
    }

    uint8_t pF = 0;
    uint8_t pR = 0;
    uint8_t pS = 0;
    uint8_t pP = 0;

    for (uint8_t i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-f") == 0) {
            pF = 1;
        } else if (strcmp(argv[i], "-r") == 0) {
            pR = 1;
        } else if (strcmp(argv[i], "-s") == 0) {
            pS = 1;
        } else if (strcmp(argv[i], "-p") == 0) {
            pP = 1;
        }
    }

    if (pF == 1) {
        printf("%d ", currentRF.forward);
        printf("%f ", currentRF.forwardWatts);
    }
    if (pR == 1) {
        printf("%d ", currentRF.reverse);
        printf("%f ", currentRF.reverseWatts);
    }
    if (pS == 1) {
        printf("%f ", currentRF.swr);
    }
    if (pP == 1) {
        printf("%d ", currentRF.frequency);
    }

    println("");
    return 0;
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
                if (!stricmp(argv[3], level_names[i])) {
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
    if (!stricmp(string, "forward") || !stricmp(string, "fwd") ||
        !stricmp(string, "f")) {
        return 0;
    }

    if (!stricmp(string, "reverse") || !stricmp(string, "rev") ||
        !stricmp(string, "r")) {
        return 1;
    }

    return atoi(string);
}

int poly(int argc, char **argv) {
    switch (argc) {
    case 1:
        println("usage: \tpoly set <array> <slot> <A> <B> <C>");
        println("\tpoly read <array> <slot>");
        return 0;
    case 4: // poly read
        if (!strcmp(argv[1], "read")) {
        } else {
            break;
        }

        char array = decode_array_number(argv[2]);
        if (array > NUMBER_OF_ARRAYS || array == -1) {
            break;
        }

        uint8_t slot = atoi(argv[3]);
        if (slot > NUMBER_OF_SLOTS) {
            break;
        }

        print_poly(&calibrationTable[array][slot]);

        return 0;
    case 7: // poly set
        if (!strcmp(argv[1], "set")) {
        } else {
            break;
        }

        char array = decode_array_number(argv[2]);
        if (array > NUMBER_OF_ARRAYS) {
            break;
        }

        uint8_t slot = atoi(argv[3]);
        if (slot > NUMBER_OF_SLOTS) {
            break;
        }

        calibrationTable[array][slot].A = atof(argv[4]);
        calibrationTable[array][slot].B = atof(argv[5]);
        calibrationTable[array][slot].C = atof(argv[6]);

        return 0;
    default:
        break;
    }
    println("invalid arguments");
    return 0;
}

/* -------------------------------------------------------------------------- */

int shell_eeprom(int argc, char **argv) {
    switch (argc) {
    case 1:
        println("usage: \teeprom write <address> <data>");
        println("\teeprom read <address>");
        return 0;

    case 3:
        if (!strcmp(argv[1], "read")) {
            LOG_TRACE({ println("eeprom read <address>"); });
            uint16_t address = atoi(argv[2]);
            LOG_DEBUG({ printf("address: %ul\r\n", (uint32_t)address); });

            printf("%02x\r\n", internal_eeprom_read(address));
            return 0;
        }
        break;

    case 4:
        if (!strcmp(argv[1], "write")) {
            LOG_TRACE({ println("eeprom write <address> <data>"); });

            uint16_t address = atoi(argv[2]);
            uint8_t data = atoi(argv[3]);

            LOG_DEBUG({ printf("address: %ul\r\n", (uint32_t)address); });

            internal_eeprom_write(address, data);
            printf("%02x\r\n", internal_eeprom_read(address));
            return 0;
        }
        break;

    default:
        break;
    }
    println("invalid arguments");
    return 0;
}

/* -------------------------------------------------------------------------- */

void print_flash_buffer(uint8_t *buffer, uint8_t element) {
    print("\t\t\t\t\t");
    for (uint8_t i = 0; i < 64; i++) {
        if (i == element) {
            printf("\033[7m%02x\033[0;37;40m ", buffer[i]);
        } else {
            printf("%02x ", buffer[i]);
        }
        if (((i + 1) % 8) == 0) {
            println("");
            print("\t\t\t\t\t");
        }
    }
    println("");
}

int shell_flash(int argc, char **argv) {
    switch (argc) {
    case 1:
        println("usage: \tflash write <address> <data>");
        println("\tflash read <address>");
        println("\tflash bread <address>");
        return 0;

    case 3:
        if (!strcmp(argv[1], "read")) {
            LOG_TRACE({ println("flash read <address>"); });
            NVM_address_t address = atoi(argv[2]);
            LOG_DEBUG({ printf("address: %ul\r\n", (uint32_t)address); });

            printf("%02x\r\n", flash_read(address));
            return 0;
        } else if (!strcmp(argv[1], "bread")) {
            LOG_TRACE({ println("flash bread <address>"); });
            NVM_address_t address = atoi(argv[2]);

            uint8_t buffer[64];
            uint8_t element = address & 0x003f;
            
            LOG_DEBUG({
                printf("address: %ul ", (uint32_t)address);
                printf("blockAddress: %ul ", (uint32_t)(address & 0xffffc0));
                printf("element: %d\r\n", element);
            });

            // Read existing block into buffer
            flash_block_read(address, buffer);

            LOG_DEBUG({
                println("buffer after read:");
                print_flash_buffer(&buffer, element);
            });
            return 0;
        }
        break;

    case 4:
        if (!strcmp(argv[1], "write")) {
            LOG_TRACE({ println("flash write <address> <data>"); });

            NVM_address_t address = atoi(argv[2]);
            uint8_t data = atoi(argv[3]);

            uint8_t buffer[64];
            uint8_t element = address & 0x003f;

            // Read existing block into buffer
            flash_block_read(address, buffer);

            LOG_DEBUG({
                printf("address: %ul ", (uint32_t)address);
                printf("blockAddress: %ul ", (uint32_t)(address & 0xffffc0));
                printf("element: %d\r\n", element);
            });

            LOG_DEBUG({
                println("buffer after read:");
                print_flash_buffer(&buffer, element);
            });

            buffer[element] = data;

            LOG_DEBUG({
                println("buffer after modification:");
                print_flash_buffer(&buffer, element);
            });

            // Write the edited buffer into flash
            flash_block_erase(address);
            flash_block_write(address, buffer);

            uint8_t verifyBuffer[64];
            flash_block_read(address, verifyBuffer);

            LOG_DEBUG({
                println("write verification:");
                print_flash_buffer(&verifyBuffer, element);
            });

            return 0;
        }
        break;

    default:
        break;
    }
    println("invalid arguments");
    return 0;
}

/* ************************************************************************** */

void register_all_shell_commands(void) {
    log_register();

    // built-in shell commands
    shell_register_command(shell_help, "help", NULL);
    shell_register_command(shell_arg_test, "test", NULL);

    // general purpose parameter touching
    shell_register_command(shell_get_param, "get", NULL);
    shell_register_command(shell_set_param, "set", NULL);

    // from display.c
    shell_register_command(shell_show_bargraphs, "bar", NULL);

    //
    shell_register_command(calibration_packet, "cal", NULL);

    // log level controls
    shell_register_command(edit_log_levels, "log", NULL);

    // calibration data
    shell_register_command(poly, "poly", NULL);

    // eeprom test
    shell_register_command(shell_eeprom, "eeprom", NULL);
    // flash test
    shell_register_command(shell_flash, "flash", NULL);
}
