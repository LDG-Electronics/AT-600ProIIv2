#include "includes.h"
#include "os/shell/shell_command_processing.h"

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

    // TODO: make the prints happen in the same order every time
    // consume each argument, printing the answers in the order requested
    for (uint8_t i = 1; i <= argc; i++) {
        if (argv[i][0] == 'f') {
            printf("%d ", currentRF.forward);
            printf("%f ", currentRF.forwardWatts);
        } else if (argv[i][0] == 'r') {
            printf("%d ", currentRF.reverse);
            printf("%f ", currentRF.reverseWatts);
        } else if (argv[i][0] == 's') {
            printf("%f ", currentRF.swr);
        } else if (argv[i][0] == 'p') {
            printf("%d ", currentRF.frequency);
        }
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

/* ************************************************************************** */

void register_all_shell_commands(void) {
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
    shell_register_command(program_log_edit, "logedit", NULL);
}
