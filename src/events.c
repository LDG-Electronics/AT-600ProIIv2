#include "includes.h"

#include "memory.h"
#include "peripherals/nonvolatile_memory.h"
#include "peripherals/pins.h"
#include "tuning.h"

/* ************************************************************************** */

void events_init(void) {

    // Calibration Event
    // event_register("swr", send_RF_data_packet, 250);
}

/* ************************************************************************** */

int16_t check_SWR_and_update_meters(void) {
    printf("check_SWR %lu\r\n", (uint32_t)systick_read());

    SWR_stable_average();

    // if (!display.displayIsLocked) {
    //     show_current_power_and_SWR();
    // }
}

/* ************************************************************************** */

void read_bypass(void) {
    printf("BP%d;\r\n", bypassStatus[system_flags.antenna]);
}

void set_bypass_off(void) {
    relays_s undoRelays;
    undoRelays = currentRelays[system_flags.antenna];

    currentRelays[system_flags.antenna] = preBypassRelays[system_flags.antenna];

    if (put_relays(&currentRelays[system_flags.antenna]) == -1) {
        currentRelays[system_flags.antenna] = undoRelays;
    }
}

void set_bypass_on(void) {
    relays_s undoRelays;
    undoRelays = currentRelays[system_flags.antenna];

    preBypassRelays[system_flags.antenna] = currentRelays[system_flags.antenna];
    currentRelays[system_flags.antenna].caps = 0;
    currentRelays[system_flags.antenna].inds = 0;
    currentRelays[system_flags.antenna].z = 0;

    if (put_relays(&bypassRelays) == -1) {
        currentRelays[system_flags.antenna] = undoRelays;
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

void read_peak(void) { printf("PK%d;\r\n", system_flags.peakMode); }
void set_peak_on(void) { system_flags.peakMode = 1; }
void set_peak_off(void) { system_flags.peakMode = 0; }
void toggle_peak(void) { system_flags.peakMode = !system_flags.peakMode; }

/* -------------------------------------------------------------------------- */

void read_scale(void) { printf("SC%d;\r\n", system_flags.scaleMode); }
void set_scale_high(void) { system_flags.scaleMode = 1; }
void set_scale_low(void) { system_flags.scaleMode = 0; }
void toggle_scale(void) { system_flags.scaleMode = !system_flags.scaleMode; }

/* -------------------------------------------------------------------------- */

void read_auto_mode(void) { printf("AT%d;\r\n", system_flags.autoMode); }
void set_auto_on(void) { system_flags.autoMode = 1; }
void set_auto_off(void) { system_flags.autoMode = 0; }
void toggle_auto(void) { system_flags.autoMode = !system_flags.autoMode; }

/* -------------------------------------------------------------------------- */

void read_hiloz(void) {
    printf("ZR%d;\r\n", currentRelays[system_flags.antenna].z);
}

void set_hiloz(uint8_t value) {
    uint8_t undo = currentRelays[system_flags.antenna].z;

    currentRelays[system_flags.antenna].z = value;

    if (put_relays(&currentRelays[system_flags.antenna]) == -1) {
        currentRelays[system_flags.antenna].z = undo;
    }
}

void set_high_z(void) { set_hiloz(1); }
void set_low_z(void) { set_hiloz(1); }
void toggle_hiloz(void) { set_hiloz(!currentRelays[system_flags.antenna].z); }

/* -------------------------------------------------------------------------- */

void read_antenna(void) { printf("AN%d;\r\n", system_flags.antenna); }

void set_antenna(uint8_t value) {
    uint8_t undo = system_flags.antenna;

    system_flags.antenna = value;

    if (put_relays(&currentRelays[system_flags.antenna]) == -1) {
        system_flags.antenna = undo;
    }

    update_antenna_LED();
}

void set_antenna_one(void) { set_antenna(1); }
void set_antenna_two(void) { set_antenna(0); }
void toggle_antenna(void) { set_antenna(!system_flags.antenna); }

/* -------------------------------------------------------------------------- */

void manual_store(void) {
    NVM_address_t address = convert_memory_address(currentRF.frequency);

    memory_store(address);

    // success animation?
    play_animation(&center_crawl);

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

/* -------------------------------------------------------------------------- */

void read_relays(void) {}

void capacitor_increment(void) {
    if (currentRelays[system_flags.antenna].caps < MAX_CAPACITORS) {
        currentRelays[system_flags.antenna].caps++;
        if (put_relays(&currentRelays[system_flags.antenna]) == -1) {
            currentRelays[system_flags.antenna].caps--;
        }
        show_cap_relays();
    } else {
        play_animation_in_background(&blink_bottom_bar_3);
    }
}

void capacitor_decrement(void) {
    if (currentRelays[system_flags.antenna].caps > MIN_CAPACITORS) {
        currentRelays[system_flags.antenna].caps--;
        if (put_relays(&currentRelays[system_flags.antenna]) == -1) {
            currentRelays[system_flags.antenna].caps++;
        }
        show_cap_relays();
    } else {
        play_animation_in_background(&blink_bottom_bar_3);
    }
}

void inductor_increment(void) {
    if (currentRelays[system_flags.antenna].inds < MAX_INDUCTORS) {
        currentRelays[system_flags.antenna].inds++;
        if (put_relays(&currentRelays[system_flags.antenna]) == -1) {
            currentRelays[system_flags.antenna].inds--;
        }
        show_ind_relays();
    } else {
        play_animation_in_background(&blink_top_bar_3);
    }
}

void inductor_decrement(void) {
    if (currentRelays[system_flags.antenna].inds > MIN_INDUCTORS) {
        currentRelays[system_flags.antenna].inds--;
        if (put_relays(&currentRelays[system_flags.antenna]) == -1) {
            currentRelays[system_flags.antenna].inds++;
        }
        show_ind_relays();
    } else {
        play_animation_in_background(&blink_top_bar_3);
    }
}

/* -------------------------------------------------------------------------- */

/*  print_RF_calibration_data()

    This function provides data to a calibration routine that runs on an LDG
    Servitor. The Servitor uses this data in conjunction with data from a
    Kenwood TS-480 radio and Alpha 4510 wattmeter to generate frequency
    compensation tables to improve the accuracy of the RF sensor.

    (F, Fw, R, Rw, SWR,      frequency)
    (0, 0,  0, 0,  0.000000, 0xffff)
*/

void print_RF_calibration_data(void) {
    printf("(%u, %f, %u, %f, %f, %u)\r\n", currentRF.forward,
           currentRF.forwardWatts, currentRF.reverse, currentRF.reverseWatts,
           currentRF.swr, currentRF.frequency);
}

#define RF_DATA_PACKET_INTERVAL 100

// This event is used to generate calibration tables
int16_t send_RF_data_packet(void) {
    SWR_stable_average();

    show_current_power_and_SWR();

    print_RF_calibration_data();

    return RF_DATA_PACKET_INTERVAL;
}