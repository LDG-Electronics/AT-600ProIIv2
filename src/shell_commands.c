#include "includes.h"
#include "os/shell/shell_command_processing.h"

/* ************************************************************************** */

void register_all_shell_commands(void) {
    // built-in shell commands
    shell_register_command(shell_help, "help", NULL);
    shell_register_command(shell_arg_test, "test", NULL);

    // general purpose parameter touching
    shell_register_command(shell_get_param, "get", NULL);
    shell_register_command(shell_set_param, "set", NULL);

    // from RF_sensor.c
    shell_register_command(shell_get_RF, "getRF", NULL);

    // from display.c
    shell_register_command(shell_show_bargraphs, "bar", NULL);
}

/* -------------------------------------------------------------------------- */
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
// parameter commands

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