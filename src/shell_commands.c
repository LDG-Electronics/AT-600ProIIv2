#include "includes.h"

/* ************************************************************************** */

command_list_t commands;

/* ************************************************************************** */

// Add a command to the command list
static void shell_register(shell_program_t program, const char *command,
                           const char *usage) {
    commands.list[commands.number].callback = program;
    commands.list[commands.number].command = command;
    commands.list[commands.number].usage = usage;

    commands.number++;
}

void shell_commands_init(void) {
    // no commands registered at startup
    commands.number = 0;

    // built-in shell commands
    shell_register(shell_help, "help", NULL);
    shell_register(shell_arg_test, "test", NULL);

    // from RF_sensor.c
    shell_register(shell_get_RF, "getRF", NULL);

    // from display.c
    shell_register(shell_show_bargraphs, "bar", NULL);

    // from relays.c
    shell_register(shell_set_relays, "setrelays", NULL);
    shell_register(shell_check_relays, "getrelays", NULL);
}

/* -------------------------------------------------------------------------- */
// built-in shell commands

int shell_help(int argc, char **argv) {
    println("-----------------------------------------------");
    shell_print_commands();
    println("-----------------------------------------------");

    return SHELL_RET_SUCCESS;
}

int shell_arg_test(int argc, char **argv) {
    println("-----------------------------------------------");
    println("SHELL ARG PARSING TEST UTILITY");
    if (argc == 1) {
        println("This command has no special arguments.");
        println("It is designed to test the TuneOS shell's arg parsing.");
        println("");
        println("Use it like this:");
        println("\"$ test command arg1 arg2 arg3\"");
        println("");
        println("To get this response:");
        println("Received 4 arguments for test command");
        println("1 - \"command\" [len:7]");
        println("2 - \"arg1\" [len:4]");
        println("3 - \"arg2\" [len:4]");
        println("4 - \"arg3\" [len:4]");
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