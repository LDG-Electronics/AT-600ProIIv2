#include "os/serial_port.h"
#include "os/shell/shell.h"
#include "os/shell/shell_command_processor.h"
#include "os/shell/shell_cursor.h"
#include "os/shell/shell_keys.h"
#include "os/shell/shell_utils.h"
#include "os/system_time.h"
#include "rf_sensor.h"
#include "shell_command_processor.h"
#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* ************************************************************************** */

void print_current_rf(void) {
    printf("FWD: %9.1f", currentRF.forwardVolts);
    print(" | ");
    printf("REV: %9.1f", currentRF.reverseVolts);
    print(" | ");
    printf("Q: %9.4f", currentRF.matchQuality);
    print(" | ");
    printf("FWatts: %9.4f", currentRF.forwardWatts);
    print(" | ");
    printf("RWatts: %9.4f", currentRF.reverseWatts);
    print(" | ");
    printf("SWR: %9.2f", currentRF.swr);
    print(" | ");
    printf("Freq: %u", currentRF.frequency);

    println("");
}

#define PRINT_COOLDOWN 1000

bool attempt_print(void) {
    static system_time_t lastAttempt = 0;
    if (time_since(lastAttempt) > PRINT_COOLDOWN) {
        lastAttempt = get_current_time();
        print_current_rf();
        return true;
    }
    return false;
}

/* ************************************************************************** */

int8_t rfmon_callback(char currentChar) {
    if (currentChar) {
        print_current_rf();
        return 0;
    }
    static bool prevIsPresent = false;

    if (prevIsPresent != currentRF.isPresent) {
        if (currentRF.isPresent) {
            println("RF is present!");
        } else {
            println("RF is absent!");
        }
    }

    if (currentRF.isPresent) {
        attempt_print();
    }

    prevIsPresent = currentRF.isPresent;

    return 0;
}

// setup
void sh_rfmon(int argc, char **argv) {
    println("entering rfmon");
    println("press any key to refresh");

    print_current_rf(); // force one print

    shell_register_callback(rfmon_callback);
}
