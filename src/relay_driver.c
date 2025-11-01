#include "relay_driver.h"
#include "os/logging.h"
#include "os/system_time.h"
#include "peripherals/pic_header.h"
#include "pins.h"
static uint8_t LOG_LEVEL = L_SILENT;

/* ************************************************************************** */

void relay_driver_init(void) {
    // set bitbang spi relay pins to default values
    set_RELAY_CLOCK_PIN(1);
    set_RELAY_DATA_PIN(1);
    set_RELAY_STROBE_PIN(1);

    log_register();
}

/* -------------------------------------------------------------------------- */

static void relay_spi_bitbang_tx_word(uint16_t word) {
    set_RELAY_STROBE_PIN(0);
    set_RELAY_CLOCK_PIN(0);

    for (uint8_t i = 0; i < 16; i++) {
        if (word & (1 << (15 - i))) {
            set_RELAY_DATA_PIN(1);
        } else {
            set_RELAY_DATA_PIN(0);
        }
        delay_us(10);
        set_RELAY_CLOCK_PIN(1);
        delay_us(10);
        set_RELAY_CLOCK_PIN(0);
        delay_us(10);
    }
    set_RELAY_STROBE_PIN(1);
    delay_us(10);
    set_RELAY_STROBE_PIN(0);
    delay_us(10);
}

void publish_relays(relay_bits_t relayBits) {
    relay_spi_bitbang_tx_word(relayBits.bits);

    // wait for the relay to stop bouncing
    delay_ms(RELAY_COIL_DELAY);
}

/* ************************************************************************** */

/*  print_relay_bits prints the contents of a relay_bits_t struct

    Format: "(<caps>, <inds>, <z>, <ant>)"
*/
void print_relay_bits(relay_bits_t relayBits) {
    printf("(C%u, L%u, Z%u, A%u)", relayBits.caps, relayBits.inds, relayBits.z,
           relayBits.ant);
}