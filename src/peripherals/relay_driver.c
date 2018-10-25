#include "relay_driver.h"
#include "../os/log_macros.h"
#include "../os/system_time.h"
#include "pic18f46k42.h"
#include "pins.h"
static uint8_t LOG_LEVEL = L_SILENT;

/* ************************************************************************** */
/*  packed_relay_bits_t is used to simplify the translation of software representation of
    relay information into a packed bit representation that's useful for
    communicating with the hardware.
*/
typedef union {
    struct {
        unsigned caps : 7;
        unsigned z : 1;
        unsigned inds : 7;
        unsigned ant : 1; // Relay is wired backwards: the off position is ANT2
    };
    uint16_t bits;
} packed_relay_bits_t;

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

void publish_relays(uint8_t caps, uint8_t inds, uint8_t z, uint8_t ant) {
    packed_relay_bits_t relayBits;
    relayBits.caps = caps;
    relayBits.inds = inds;
    relayBits.z = z;
    relayBits.ant = ant;

    relay_spi_bitbang_tx_word(relayBits.bits);

    // wait for the relay to stop bouncing
    delay_ms(RELAY_COIL_DELAY);
}