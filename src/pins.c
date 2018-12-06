#include "pins.h"
#include "peripherals/pic_header.h"

/* ************************************************************************** */

void pins_init(void) {
    // RF Sensor
    TRISAbits.TRISA0 = 1; // FWD_PIN
    TRISAbits.TRISA1 = 1; // REV_PIN
#ifdef DEVELOPMENT
    TRISEbits.TRISE0 = 1; // FREQ_PIN
#else
    TRISEbits.TRISE3 = 1; // FREQ_PIN
#endif

    // Front panel buttons
    TRISAbits.TRISA3 = 1; // POWER_BUTTON
    TRISAbits.TRISA4 = 1; // CDN_BUTTON
    TRISAbits.TRISA5 = 1; // LUP_BUTTON
    TRISBbits.TRISB2 = 1; // CUP_BUTTON
    TRISBbits.TRISB4 = 1; // FUNC_BUTTON
    TRISBbits.TRISB5 = 1; // LDN_BUTTON

#ifdef DEVELOPMENT
    TRISEbits.TRISE2 = 1; // ANT_BUTTON
    TRISEbits.TRISE1 = 1; // TUNE_BUTTON
#else
    TRISBbits.TRISB6 = 1; // ANT_BUTTON
    TRISBbits.TRISB7 = 1; // TUNE_BUTTON
#endif

    // Front Panel bitbang SPI
    TRISAbits.TRISA6 = 0; // FP_CLOCK_PIN
    TRISCbits.TRISC4 = 0; // FP_STROBE_PIN
    TRISCbits.TRISC5 = 0; // FP_DATA_PIN

    // Front Panel LEDs
    TRISAbits.TRISA2 = 0; // POWER_LED_PIN
    TRISBbits.TRISB1 = 0; // ANT_LED_PIN
    TRISCbits.TRISC3 = 0; // BYPASS_LED_PIN

    // Relay driver bitbang SPI
    TRISCbits.TRISC0 = 0; // RELAY_CLOCK_PIN
    TRISCbits.TRISC1 = 0; // RELAY_DATA_PIN
    TRISCbits.TRISC2 = 0; // RELAY_STROBE_PIN

    // Radio Command Pin
    TRISAbits.TRISA7 = 0; // RADIO_CMD_PIN

    // Meter port pins
    TRISCbits.TRISC6 = 0; // Meter TX
    TRISCbits.TRISC7 = 1; // Meter RX

    // Debug UART pins
    TRISDbits.TRISD2 = 0; // Debug TX
    TRISDbits.TRISD3 = 1; // Debug RX

    // SWR sensor analog enabled
    ANSELAbits.ANSELA0 = 1; // FWD_PIN
    ANSELAbits.ANSELA1 = 1; // REV_PIN

    // Front panel pullups
    WPUAbits.WPUA3 = 1; // POWER_BUTTON
    WPUAbits.WPUA4 = 1; // CDN_BUTTON
    WPUAbits.WPUA5 = 1; // LUP_BUTTON
    WPUBbits.WPUB2 = 1; // CUP_BUTTON
    WPUBbits.WPUB4 = 1; // FUNC_BUTTON
    WPUBbits.WPUB5 = 1; // LDN_BUTTON

#ifdef DEVELOPMENT
    WPUEbits.WPUE2 = 1; // ANT_BUTTON
    WPUEbits.WPUE1 = 1; // TUNE_BUTTON
#else
    WPUBbits.WPUB6 = 1;   // ANT_BUTTON
    WPUBbits.WPUB7 = 1;   // TUNE_BUTTON
#endif
}