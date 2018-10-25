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

void publish_relays(uint16_t word) {

    relay_spi_bitbang_tx_word(word);
}