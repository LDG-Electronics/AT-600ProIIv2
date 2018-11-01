#include "events.h"
#include "flags.h"
#include "memory.h"
#include "os/serial_port.h"
#include "os/system_time.h"
#include "peripherals/pic18f46k42.h"
#include "peripherals/pins.h"
#include "relays.h"
#include "tuning.h"

/* ************************************************************************** */

void set_bypass_off(void) {
    if (put_relays(&preBypassRelays[systemFlags.antenna]) == -1) {
        // TODO: what do we do on relayerror?
    }
}

void set_bypass_on(void) {
    // save current relays into preBypassRelays
    preBypassRelays[systemFlags.antenna] = read_current_relays();

    if (put_relays(&bypassRelays) == -1) {
        // TODO: what do we do on relayerror?
    }
}

void toggle_bypass(void) {
    if (bypassStatus[systemFlags.antenna] == 1) {
        set_bypass_off();
    } else {
        set_bypass_on();
    }
}

/* -------------------------------------------------------------------------- */

void set_peak_on(void) { systemFlags.peakMode = 1; }
void set_peak_off(void) { systemFlags.peakMode = 0; }
void toggle_peak(void) { systemFlags.peakMode = !systemFlags.peakMode; }

/* -------------------------------------------------------------------------- */

void set_scale_high(void) { systemFlags.scaleMode = 1; }
void set_scale_low(void) { systemFlags.scaleMode = 0; }
void toggle_scale(void) { systemFlags.scaleMode = !systemFlags.scaleMode; }

/* -------------------------------------------------------------------------- */

void set_auto_on(void) { systemFlags.autoMode = 1; }
void set_auto_off(void) { systemFlags.autoMode = 0; }
void toggle_auto(void) { systemFlags.autoMode = !systemFlags.autoMode; }

/* -------------------------------------------------------------------------- */

void set_hiloz(uint8_t value) {
    relays_t relays = read_current_relays();

    relays.z = value;

    if (put_relays(&relays) == -1) {
        // TODO: what do we do on relayerror?
    }
}

void set_high_z(void) { set_hiloz(1); }
void set_low_z(void) { set_hiloz(0); }
void toggle_hiloz(void) { set_hiloz(!currentRelays[systemFlags.antenna].z); }

/* -------------------------------------------------------------------------- */

void set_antenna(uint8_t value) {
    systemFlags.antenna = value;
    relays_t relays = read_current_relays();

    if (put_relays(&relays) == -1) {
        // TODO: what do we do on relayerror?
    }
}

void set_antenna_one(void) { set_antenna(1); }
void set_antenna_two(void) { set_antenna(0); }
void toggle_antenna(void) { set_antenna(!systemFlags.antenna); }

/* -------------------------------------------------------------------------- */

void manual_store(void) {
    // memory_store(convert_memory_address(currentRF.frequency));

    // success animation?
    // play_animation(&center_crawl[0]);

    // failure animation?
    // repeat_animation(&blink_both_bars, 2);
}

/* -------------------------------------------------------------------------- */
void set_power_status(uint8_t value) {
    systemFlags.powerStatus = value;

    if (systemFlags.powerStatus == 1) {
        put_relays(&currentRelays[systemFlags.antenna]);
    } else {
        relays_t temp = read_current_relays();
        put_relays(&bypassRelays);
        currentRelays[systemFlags.antenna] = temp;
    }
}

void set_power_on(void) { set_power_status(1); }
void set_power_off(void) { set_power_status(0); }

void toggle_power_status(void) { set_power_status(!systemFlags.powerStatus); }

/* -------------------------------------------------------------------------- */

void request_memory_tune(void) {
    RADIO_CMD_PIN = 1;

    memory_tune();

    if (tuning_flags.noMemory == 1) {
        full_tune();
    }

    RADIO_CMD_PIN = 0;
    tuning_followup_animation();
}

void request_full_tune(void) {
    RADIO_CMD_PIN = 1;

    full_tune();

    RADIO_CMD_PIN = 0;
    tuning_followup_animation();
}