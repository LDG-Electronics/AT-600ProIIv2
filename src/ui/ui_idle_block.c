#include "ui_idle_block.h"
#include "display.h"
#include "events.h"
#include "flags.h"
#include "os/buttons.h"
#include "os/serial_port.h"
#include "os/shell/shell.h"
#include "os/system_time.h"
#include "os/usb_port.h"
#include "relays.h"
#include "rf_sensor.h"
#include "ui.h"
#include "ui_bargraphs.h"
#include "usb/messages.h"
#include "usb_port.h"
#include <stdbool.h>

/* ************************************************************************** */

#define RF_POLLS_PER_SECOND 100
#define RF_POLL_COOLDOWN 1000 / RF_POLLS_PER_SECOND

bool attempt_RF_poll(void) {
    static system_time_t lastAttempt = 0;
    if (time_since(lastAttempt) < RF_POLL_COOLDOWN) {
        return false;
    }
    lastAttempt = get_current_time();

    poll_RF(); // takes 203 uS
    return true;
}

/* -------------------------------------------------------------------------- */

#define FREQUENCY_UPDATES_PER_SECOND 1
#define FREQUENCY_UPDATE_PERIOD 1000 / FREQUENCY_UPDATES_PER_SECOND

bool attempt_frequency_measurement(void) {
    static system_time_t lastAttempt = 0;
    if (time_since(lastAttempt) < FREQUENCY_UPDATE_PERIOD) {
        return false;
    }
    lastAttempt = get_current_time();

    measure_frequency(); // ~2500uS @ 50MHz, ~60000uS @ 1.8MHz
    return true;
}

/* -------------------------------------------------------------------------- */

#define RF_UPDATES_PER_SECOND 50
#define RF_UPDATE_COOLDOWN 1000 / RF_UPDATES_PER_SECOND

bool attempt_RF_measurement(void) {
    static system_time_t lastAttempt = 0;
    if (time_since(lastAttempt) < RF_UPDATE_COOLDOWN) {
        return false;
    }
    lastAttempt = get_current_time();

    measure_RF(); // ~1700uS
    return true;
}

/* -------------------------------------------------------------------------- */

#if defined DEVELOPMENT && defined USB_ENABLED
static uint16_t calculate_cooldown(void) {
    if (!currentRF.isPresent) {
        return 500;
    }
    return 50;
}

static bool attempt_RF_message(void) {
    static system_time_t lastAttempt = 0;
    static bool updating = false;
    if (updating && judi_is_recieving()) {
        updating = false;
        lastAttempt = get_current_time();
        return false;
    }

    if (time_since(lastAttempt) < calculate_cooldown()) {
        return false;
    }

    if (!updating) {
        if (time_since(lastAttempt) < 1000) {
            return false;
        } else {
            updating = true;
        }
    }
    lastAttempt = get_current_time();

    json_print(usb_print, rfUpdate);

    return true;
}
#endif

/* -------------------------------------------------------------------------- */

#define BARGRAPH_UPDATES_PER_SECOND 30
#define BARGRAPH_UPDATE_COOLDOWN 1000 / BARGRAPH_UPDATES_PER_SECOND

bool attempt_bargraph_update(void) {
    static system_time_t lastAttempt = 0;
    if (time_since(lastAttempt) < BARGRAPH_UPDATE_COOLDOWN) {
        return false;
    }
    lastAttempt = get_current_time();

    calculate_watts_and_swr(); // ~3800uS
    update_bargraphs();        // ~180uS
    return true;
}

/* -------------------------------------------------------------------------- */

#define FLAG_SAVE_COOLDOWN 200

bool attempt_flag_save(void) {
    static system_time_t lastAttempt = 0;
    if (time_since(lastAttempt) < FLAG_SAVE_COOLDOWN) {
        return false;
    }
    lastAttempt = get_current_time();

    save_flags(); // takes either 80uS or 28mS
    return true;
}

/* -------------------------------------------------------------------------- */

bool allowedToAutoTune = true;

void disable_auto_tuning(void) { allowedToAutoTune = false; }
void enable_auto_tuning(void) { allowedToAutoTune = true; }

#define AUTO_TUNE_COOLDOWN 200

bool attempt_auto_tune(void) {
    static system_time_t lastAttempt = 0;
    if (time_since(lastAttempt) < AUTO_TUNE_COOLDOWN) {
        return false;
    }
    lastAttempt = get_current_time();

    if (systemFlags.autoMode && allowedToAutoTune) {
        if (currentRF.swr > get_SWR_threshold()) {
            disable_auto_tuning();
            request_memory_tune();
            return true;
        }
    }
    return false;
}

/* -------------------------------------------------------------------------- */

void ui_idle_block(void) {
    // TODO: profile me
    if (attempt_RF_poll()) {
        return;
    }

    if (RF_is_present()) {
        // ~2500uS @ 50MHz, ~60000uS @ 1.8MHz
        if (attempt_frequency_measurement()) {
            return;
        }
        // ~1700uS
        if (attempt_RF_measurement()) {
            return;
        }
        // TODO: profile me
        if (attempt_auto_tune()) {
            return;
        }
    }

    if (!RF_is_present()) {
        enable_auto_tuning(); // reset when radio is unkeyed
    }

    // ~4000uS
    if (attempt_bargraph_update()) {
        return;
    }

#ifdef DEVELOPMENT
    // ~22uS, most shell commands are ~2000uS
    shell_update((char)getch());
    #ifdef USB_ENABLED
    judi_update(usb_getch());

    // if (attempt_RF_message()) {
    //     return;
    // }
    #endif
#endif

    // ~30mS
    if (attempt_flag_save()) {
        return;
    }
}
