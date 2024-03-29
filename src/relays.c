#include "relays.h"
#include "display.h"
#include "flags.h"
#include "os/logging.h"
#include "os/system_time.h"
#include "rf_sensor.h"
static uint8_t LOG_LEVEL = L_SILENT;

/* ************************************************************************** */

// Global
relays_t currentRelays[NUM_OF_ANTENNA_PORTS];
relays_t bypassRelays;
relays_t preBypassRelays[NUM_OF_ANTENNA_PORTS];

/* ************************************************************************** */

void relays_init(void) {
    // Initialize relay structs
    currentRelays[0].all = 0;
    currentRelays[0].ant = 0;
    currentRelays[1].all = 0;
    currentRelays[1].ant = 1;
    bypassRelays.all = 0;
    preBypassRelays[0].all = 0;
    preBypassRelays[0].ant = 0;
    preBypassRelays[1].all = 0;
    preBypassRelays[1].ant = 1;

    relay_driver_init();

    log_register();
}

/* -------------------------------------------------------------------------- */

relay_bits_t pack_relays(relays_t relays) {
    relay_bits_t relayBits;

    relayBits.caps = relays.caps;
    relayBits.inds = relays.inds;
    relayBits.z = relays.z;
    relayBits.ant = relays.ant;

    return relayBits;
}

relays_t unpack_relays(relay_bits_t relayBits) {
    relays_t relays;

    relays.caps = relayBits.caps;
    relays.inds = relayBits.inds;
    relays.z = relayBits.z;
    relays.ant = relayBits.ant;

    return relays;
}

/* -------------------------------------------------------------------------- */
/*  check_if_safe() takes an SWR measurement and determines if it's too
    dangerous to switch relays. Relays should not be touched above 100W or 125W.

    If it's not safe to switch relays, the current operation should be halted
    immediately, and the 'railroad crossing lights' animation should be played
    on the front panel.
*/
int8_t check_if_safe(void) {
    measure_RF();
    calculate_watts_and_swr();

    if (currentRF.forwardWatts > 125) {
        return -1;
    }

    return 0;
}

void update_bypass_status(relays_t relays) {
    systemFlags.bypassStatus[systemFlags.antenna] = 0;
    if ((relays.caps == 0) && (relays.inds == 0)) {
        systemFlags.bypassStatus[systemFlags.antenna] = 1;
    }

    update_status_LEDs();
}

/*  put_relays() takes a pointer to a relay struct and attempts to publish that
    struct to the physical relays.

*/
int8_t put_relays(relays_t relays) {
    LOG_TRACE({ println("put_relays"); });

    // overwrite the antenna setting just in case it got clobbered by something
    relays.ant = systemFlags.antenna;

    LOG_INFO({
        print("new relays: ");
        print_relays(relays);
        print(", old relays: ");
        print_relays(currentRelays[systemFlags.antenna]);
        println("");
    });

    // make sure the radio isn't transmitting too much power
    // this is EXTRA important on >100W tuners
    if (check_if_safe() == -1) {
        LOG_ERROR({ println("not safe to switch relays"); });
        return (-1);
    }

    // calculate whether the new relays count as bypass
    update_bypass_status(relays);

    // pass off the new relays to the relay driver
    relay_bits_t relayBits = pack_relays(relays);
    publish_relays(relayBits);

    // Update the global bulletin board
    currentRelays[systemFlags.antenna] = relays;

    return 0;
}

/* ************************************************************************** */

/*  print_relays prints the contents of a relays_t struct

    Format: "(<caps>, <inds>, <z>, <ant>)"
*/
void print_relays(relays_t relays) {
    printf("(C%d, L%d, Z%d, A%d)", relays.caps, relays.inds, relays.z,
           relays.ant);
}