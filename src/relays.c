#include "includes.h"
#include "relays.h"
#include "pins.h"
#include "delay.h"
#include "display.h"

/* ************************************************************************** */

// Global
relays_s currentRelays[NUM_OF_ANTENNA_PORTS];
relays_s bypassRelays; //TODO: needs to be const, and permanently equal {0, 0, 0}
relays_s preBypassRelays[NUM_OF_ANTENNA_PORTS];

/* ************************************************************************** */

void relays_init(void)
{
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

    // 
    RELAY_STROBE_PIN = 1;
    RELAY_CLOCK_PIN = 1;
    RELAY_DATA_PIN = 1;

    // Setup shell commands
    shell_register(shell_set_relays, "setrelays");
    shell_register(shell_check_relays, "getrelays");
}

/* -------------------------------------------------------------------------- */

void publish_relays(uint16_t word)
{
    uint8_t i;

    RELAY_STROBE_PIN = 1;
    RELAY_CLOCK_PIN = 0;

    for (i = 0; i < 16; i++)
    {
        if (word & (1 << (15 - i))) {
            RELAY_DATA_PIN = 1;
        } else {
            RELAY_DATA_PIN = 0;
        }
        delay_us(10);
        RELAY_CLOCK_PIN = 1;
        delay_us(10);
        RELAY_CLOCK_PIN = 0;
        delay_us(10);
    }
    RELAY_STROBE_PIN = 0;
    delay_us(10);
    RELAY_STROBE_PIN = 1;
    delay_us(10);
}

/* -------------------------------------------------------------------------- */
/*  check_if_safe() takes an SWR measurement and determines if it's too
    dangerous to switch relays. Relays should not be touched above 100W or 125W.

    If it's not safe to switch relays, the current operation should be halted
    immediately, and the 'railroad crossing lights' animation should be played
    on the front panel.
*/
int8_t check_if_safe(void)
{
    SWR_average();
    
    return 0;
}

void update_bypass_status(relays_s *testRelays)
{
    bypassStatus[system_flags.antenna] = 0;
    if ((testRelays->caps == 0) && 
        (testRelays->z == 0) && 
        (testRelays->inds == 0)) {
            
        bypassStatus[system_flags.antenna] = 1;
    }

    update_bypass_LED();
}

/*  put_relays() takes a pointer to a relay struct and attempts to publish that
    struct to the physical relays.
    
*/
int8_t put_relays(relays_s *testRelays)
{
    if (check_if_safe() == -1) return (-1);
    
    update_bypass_status(testRelays);
    
    publish_relays(testRelays->all);

    delay_ms(RELAY_COIL_DELAY);

    return 0;
}

/* -------------------------------------------------------------------------- */

// Prints the contents of relay struct as "(caps, inds, z)"
void print_relays(relays_s *relays)
{
    printf("(C%d, L%d, Z%d, A%d)", relays->caps, relays->inds, relays->z, relays->ant);
}

// Same as log_relays(), but also appends a CRLF.
void print_relays_ln(relays_s *relays)
{
    print_relays(relays);
    
    print_ln();
}

int shell_set_relays(int argc, char** argv)
{
    return SHELL_RET_SUCCESS;
}

int shell_check_relays(int argc, char** argv)
{
    print_relays(&currentRelays[system_flags.antenna]);

    return SHELL_RET_SUCCESS;
}