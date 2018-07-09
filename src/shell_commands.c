#include "includes.h"

/* ************************************************************************** */

command_list_t commands;

void shell_commands_init(void) {

    // from RF_sensor.c
    shell_register(shell_get_RF, "getRF");

    // from display.c
    shell_register(shell_show_bargraphs, "bar");

    // from relays.c
    shell_register(shell_set_relays, "setrelays");
    shell_register(shell_check_relays, "getrelays");

    // from shell.c
    shell_register(shell_help, "help");
    shell_register(shell_test, "test");
}

// Add a command to the command list
bool shell_register(shell_program_t program, const char *string) {
    unsigned char i;

    for (i = 0; i < MAXIMUM_NUM_OF_SHELL_COMMANDS; i++) {
        if (commands.list[i].callback != 0 || commands.list[i].command != 0)
            continue;
        commands.list[i].callback = program;
        commands.list[i].command = string;
        return true;
    }
    return false;
}

/* -------------------------------------------------------------------------- */
// from shell.c

int shell_help(int argc, char **argv) {
    shell_print_commands();

    return SHELL_RET_SUCCESS;
}

int shell_test(int argc, char **argv) {
    println("-----------------------------------------------");
    println("SHELL DEBUG / TEST UTILITY");
    println("-----------------------------------------------");
    println("");
    printf("Received %d arguments for test command\r\n", argc);

    // Print each argument with string lengths
    for (uint8_t i = 0; i < argc; i++) {
        // Print formatted text to terminal
        // shell_printf("%d - \"%s\" [len:%d]\r\n", i, argv[i], strlen(argv[i])
        // );
        println(argv[i]);
    }

    return SHELL_RET_SUCCESS;
}

/* -------------------------------------------------------------------------- */
// from RF_sensor.c

int shell_get_RF(int argc, char **argv) {
    if (argc == 1) {
        print_current_SWR_ln();
    } else {
        if (!strcmp(argv[1], "-fwd")) {
            printf("%d\r\n", currentRF.forward);
        } else if (!strcmp(argv[1], "-rev")) {
            printf("%d\r\n", currentRF.reverse);
        } else if (!strcmp(argv[1], "-swr")) {
            printf("%f\r\n", currentRF.swr);
        } else if (!strcmp(argv[1], "-freq")) {
            printf("%d\r\n", currentRF.frequency);
        } else {
            println("invalid argument");
        }
    }

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
// from relays.c

int shell_set_relays(int argc, char **argv) { return SHELL_RET_SUCCESS; }

int shell_check_relays(int argc, char **argv) {
    print_relays(&currentRelays[system_flags.antenna]);

    return SHELL_RET_SUCCESS;
}