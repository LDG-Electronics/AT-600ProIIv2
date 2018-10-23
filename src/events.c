#include "events.h"
#include "flags.h"
#include "memory.h"
#include "os/console_io.h"
#include "os/system_time.h"
#include "peripherals/pic18f46k42.h"
#include "peripherals/pins.h"
#include "relays.h"
#include "tuning.h"

/* ************************************************************************** */

void set_bypass_off(void) {
    if (put_relays(&preBypassRelays[system_flags.antenna]) == -1) {
        // TODO: what do we do on relayerror?
    }
}

void set_bypass_on(void) {
    // save current relays into preBypassRelays
    preBypassRelays[system_flags.antenna] = read_current_relays();

    if (put_relays(&bypassRelays) == -1) {
        // TODO: what do we do on relayerror?
    }
}

void toggle_bypass(void) {
    if (bypassStatus[system_flags.antenna] == 1) {
        set_bypass_off();
    } else {
        set_bypass_on();
    }
}

/* -------------------------------------------------------------------------- */

void set_peak_on(void) { system_flags.peakMode = 1; }
void set_peak_off(void) { system_flags.peakMode = 0; }
void toggle_peak(void) { system_flags.peakMode = !system_flags.peakMode; }

/* -------------------------------------------------------------------------- */

void set_scale_high(void) { system_flags.scaleMode = 1; }
void set_scale_low(void) { system_flags.scaleMode = 0; }
void toggle_scale(void) { system_flags.scaleMode = !system_flags.scaleMode; }

/* -------------------------------------------------------------------------- */

void set_auto_on(void) { system_flags.autoMode = 1; }
void set_auto_off(void) { system_flags.autoMode = 0; }
void toggle_auto(void) { system_flags.autoMode = !system_flags.autoMode; }

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
void toggle_hiloz(void) { set_hiloz(!currentRelays[system_flags.antenna].z); }

/* -------------------------------------------------------------------------- */

void set_antenna(uint8_t value) {
    system_flags.antenna = value;
    relays_t relays = read_current_relays();

    if (put_relays(&relays) == -1) {
        // TODO: what do we do on relayerror?
    }
}

void set_antenna_one(void) { set_antenna(1); }
void set_antenna_two(void) { set_antenna(0); }
void toggle_antenna(void) { set_antenna(!system_flags.antenna); }

/* -------------------------------------------------------------------------- */

void manual_store(void) {
    // NVM_address_t address = convert_memory_address(currentRF.frequency);

    // memory_store(address);

    // success animation?
    // play_animation(&center_crawl[0]);

    // failure animation?
    // repeat_animation(&blink_both_bars, 2);
}

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