#include "events.h"
#include "flags.h"
#include "tuning/tuning_memories.h"
#include "os/serial_port.h"
#include "os/system_time.h"
#include "peripherals/pic_header.h"
#include "pins.h"
#include "relays.h"
#include "rf_sensor.h"
#include "tuning.h"

/* ************************************************************************** */

void set_bypass_on(void) {
    // save current relays into preBypassRelays
    preBypassRelays[systemFlags.antenna] = read_current_relays();

    if (put_relays(bypassRelays) == -1) {
        // TODO: what do we do on relayerror?
    }
}

void set_bypass_off(void) {
    if (put_relays(preBypassRelays[systemFlags.antenna]) == -1) {
        // TODO: what do we do on relayerror?
    }
}

void toggle_bypass(void) {
    if (systemFlags.bypassStatus[systemFlags.antenna] == 1) {
        set_bypass_off();
    } else {
        set_bypass_on();
    }
}

/* -------------------------------------------------------------------------- */

void set_peak_mode(uint8_t value) { systemFlags.peakMode = value; }
void set_peak_on(void) { systemFlags.peakMode = 1; }
void set_peak_off(void) { systemFlags.peakMode = 0; }
void toggle_peak(void) { systemFlags.peakMode = !systemFlags.peakMode; }

/* -------------------------------------------------------------------------- */

void set_scale_mode(uint8_t value) { systemFlags.scaleMode = value; }
void set_scale_high(void) { systemFlags.scaleMode = 1; }
void set_scale_low(void) { systemFlags.scaleMode = 0; }
void toggle_scale(void) { systemFlags.scaleMode = !systemFlags.scaleMode; }

/* -------------------------------------------------------------------------- */

void set_auto_mode(uint8_t value) { systemFlags.autoMode = value; }
void set_auto_on(void) { systemFlags.autoMode = 1; }
void set_auto_off(void) { systemFlags.autoMode = 0; }
void toggle_auto(void) { systemFlags.autoMode = !systemFlags.autoMode; }

/* -------------------------------------------------------------------------- */

void set_hiloz(uint8_t value) {
    relays_t relays = read_current_relays();

    relays.z = value;

    if (put_relays(relays) == -1) {
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

    if (put_relays(relays) == -1) {
        // TODO: what do we do on relayerror?
    }
}

void set_antenna_one(void) { set_antenna(1); }
void set_antenna_two(void) { set_antenna(0); }
void toggle_antenna(void) { set_antenna(!systemFlags.antenna); }

/* -------------------------------------------------------------------------- */

/*  // TODO: this is completely fubar
    It needs...
    * animations
    * success/failure
    * how to handle if currentRF.frequency is invalid
*/
void manual_store(void) {
    // relays_t relays = read_current_relays();
    // NVM_address_t address = convert_memory_address(currentRF.frequency);
    // if (address) {
    //     memory_store(address, relays);
    // }

    // success animation?
    // play_animation(&center_crawl[0]);

    // failure animation?
    // repeat_animation(&blink_both_bars, 2);
}

/* -------------------------------------------------------------------------- */

void set_power_on(void) {
    systemFlags.powerStatus = 1;
    
    if (put_relays(currentRelays[systemFlags.antenna]) == -1) {
        // TODO: what do we do on relayerror?
    }
}

void set_power_off(void) {
    systemFlags.powerStatus = 0;

    // put_relays() always publishes its argument to currentRelays
    // We actually don't want that behavior here, so save currentRelays and
    // restore it after it gets overwritten.
    relays_t temp = read_current_relays();
    if (put_relays(bypassRelays) == -1) {
        // TODO: what do we do on relayerror?
    }
    currentRelays[systemFlags.antenna] = temp;
}

/* -------------------------------------------------------------------------- */

void request_memory_tune(void) {
    // key the radio
    set_RADIO_CMD_PIN(1);

    // first, attempt to recall an appropriate memory from storage
    tuning_errors_t errors = memory_tune();

    // if we didn't find a good memory, but nothing else went wrong, then start
    // from scratch with a full tune
    if (errors.noMemory == 1) {
        errors = full_tune();
    }

    // unkey the radio
    set_RADIO_CMD_PIN(0);
    tuning_followup_animation(errors);
}

void request_full_tune(void) {
    // key the radio
    set_RADIO_CMD_PIN(1);

    // do the thing
    tuning_errors_t errors = full_tune();

    // unkey the radio
    set_RADIO_CMD_PIN(0);
    tuning_followup_animation(errors);
}