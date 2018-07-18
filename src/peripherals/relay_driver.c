#include "../includes.h"

#include "pins.h"
static uint8_t LOG_LEVEL = L_SILENT;

/* ************************************************************************** */

// TODO: do I need to write a generic bitbang SPI driver to fit under this file?
void relay_driver_init(void) {
    // set bitbang spi relay pins to default values
    RELAY_STROBE_PIN = 1;
    RELAY_CLOCK_PIN = 1;
    RELAY_DATA_PIN = 1;

    log_register();
}

/* -------------------------------------------------------------------------- */

void publish_relays(uint16_t word) {
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