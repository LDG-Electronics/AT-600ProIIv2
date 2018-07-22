#include "includes.h"

#include "os/log_macros.h"
#include "peripherals/relay_driver.h"
static uint8_t LOG_LEVEL = L_SILENT;

/* ************************************************************************** */

// Global
relays_s currentRelays[NUM_OF_ANTENNA_PORTS];
relays_s bypassRelays;
relays_s preBypassRelays[NUM_OF_ANTENNA_PORTS];

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

void update_bypass_status(relays_s *testRelays) {
    bypassStatus[system_flags.antenna] = 0;
    if ((testRelays->caps == 0) && (testRelays->inds == 0)) {

        bypassStatus[system_flags.antenna] = 1;
    }

    update_bypass_LED();
}

/*  put_relays() takes a pointer to a relay struct and attempts to publish that
    struct to the physical relays.

*/
int8_t put_relays(relays_s *testRelays) {
    if (check_if_safe() == -1)
        return (-1);

    update_bypass_status(testRelays);

    publish_relays(testRelays->all);

    delay_ms(RELAY_COIL_DELAY);

    return 0;
}

/* -------------------------------------------------------------------------- */

// Prints the contents of relay struct as "(caps, inds, z)"
void print_relays(relays_s *relays) {
    printf("(C%d, L%d, Z%d, A%d)", relays->caps, relays->inds, relays->z,
           relays->ant);
}

// Same as log_relays(), but also appends a CRLF.
void print_relays_ln(relays_s *relays) {
    print_relays(relays);

    println("");
}
