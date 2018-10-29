#include "relay_driver.h"
#include "../os/log_macros.h"
#include "../os/system_time.h"
#include "pic18f46k42.h"
#include "pins.h"
static uint8_t LOG_LEVEL = L_SILENT;

/* ************************************************************************** */

void relay_driver_init(void) {
    // set bitbang spi relay pins to default values
    RELAY_STROBE_PIN = 1;
    RELAY_CLOCK_PIN = 1;
    RELAY_DATA_PIN = 1;

    log_register();
}

/* -------------------------------------------------------------------------- */

static void relay_spi_bitbang_tx_word(uint16_t word) {
    RELAY_STROBE_PIN = 1;
    RELAY_CLOCK_PIN = 0;

    for (uint8_t i = 0; i < 16; i++) {
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

void publish_relays(packed_relays_t *relayBits) {
    relay_spi_bitbang_tx_word(relayBits->bits);

    // wait for the relay to stop bouncing
    delay_ms(RELAY_COIL_DELAY);
}

/* ************************************************************************** */

/*  print_relay_bits prints the contents of a packed_relays_t struct

    Format: "(<caps>, <inds>, <z>, <ant>)"
*/
void print_relay_bits(packed_relays_t *relayBits) {
    printf("(C%d, L%d, Z%d, A%d)", relayBits->caps, relayBits->inds,
           relayBits->z, relayBits->ant);
}