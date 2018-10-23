#include "relays.h"
#include "display.h"
#include "flags.h"
#include "os/log_macros.h"
#include "os/system_time.h"
#include "peripherals/relay_driver.h"
#include "rf_sensor.h"
static uint8_t LOG_LEVEL = L_SILENT;

/* ************************************************************************** */

// Global
relays_t currentRelays[NUM_OF_ANTENNA_PORTS];
relays_t bypassRelays;
relays_t preBypassRelays[NUM_OF_ANTENNA_PORTS];

// Prints the contents of relay struct as "(caps, inds, z)"
void print_relays(relays_t *relays) {
    printf("(C%d, L%d, Z%d, A%d)", relays->caps, relays->inds, relays->z,
           relays->ant);
}
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
/*  check_if_safe() takes an SWR measurement and determines if it's too
    dangerous to switch relays. Relays should not be touched above 100W or 125W.

    If it's not safe to switch relays, the current operation should be halted
    immediately, and the 'railroad crossing lights' animation should be played
    on the front panel.
*/
int8_t check_if_safe(void) {
    SWR_average();

    return 0;
}

void update_bypass_status(relays_t *relays) {
    bypassStatus[system_flags.antenna] = 0;
    if ((relays->caps == 0) && (relays->inds == 0)) {
        bypassStatus[system_flags.antenna] = 1;
    }

    update_bypass_LED();
}

/*  put_relays() takes a pointer to a relay struct and attempts to publish that
    struct to the physical relays.

*/
int8_t put_relays(relays_t *relays) {
    LOG_TRACE({ println("put_relays"); });

    // overwrite the antenna setting just in case it got clobbered by something
    relays->ant = system_flags.antenna;

    LOG_INFO({
        print("new relays: ");
        print_relays(relays);
        print(", old relays: ");
        print_relays(&currentRelays[system_flags.antenna]);
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
    publish_relays(relays->all);

    // Update the global bulletin board
    currentRelays[system_flags.antenna] = *relays;

    // wait for the relay to stop bouncing
    delay_ms(RELAY_COIL_DELAY);

    return 0;
}
