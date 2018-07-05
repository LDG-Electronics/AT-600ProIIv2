#include "../includes.h"

/* ************************************************************************** */

void port_init(void)
{
    // Tri-state control; 0 = output enabled, 1 = output disabled
    // Explicitly disable all outputs
    TRISA = 0xFF;
    TRISB = 0xFF;
    TRISC = 0xFF;
    TRISD = 0xFF;
    TRISE = 0xFF;

    // Output latch - explicitly drive all outputs low
    LATA = 0;    
    LATB = 0;    
    LATC = 0;   
    LATD = 0;   
    LATE = 0;   
    
    // Analog Select; 0 = analog mode is disabled, 1 = analog mode is enabled
    ANSELA = 0;
    ANSELB = 0;
    ANSELC = 0;
    ANSELD = 0;
    ANSELE = 0;

    // Weak Pull-up; 0 = pull-up disabled, 1 = pull-up enabled
    WPUA = 0;
    WPUB = 0;
    WPUC = 0;
    WPUD = 0;
    WPUE = 0;

    // Open-Drain Control; 0 = Output drives both high and low, 1 = 
    ODCONA = 0;
    ODCONB = 0;
    ODCONC = 0;
    ODCOND = 0;
    ODCONE = 0;

    // Slew Rate Control; 0 = maximum slew rate, 1 = limited slew rate
    SLRCONA = 0;
    SLRCONB = 0;
    SLRCONC = 0;
    SLRCOND = 0;
    SLRCONE = 0;

    // Input (Logic) Level Control; 0 = TTL, 1 = Schmitt Trigger
    INLVLA = 0xff;
    INLVLB = 0xff;
    INLVLC = 0xff;
    INLVLD = 0xff;
    INLVLE = 0xff;
}

void pins_init(void)
{
    // initialize all the port registers
    port_init();

    // RF Sensor
    TRISAbits.TRISA0 = 1; // FWD_PIN
    TRISAbits.TRISA1 = 1; // REV_PIN
    TRISBbits.TRISB0 = 1; // FREQ_PIN <- temporary pin for development purposes
    // TRISEbits.TRISE0 = 1; // FREQ_PIN

    // Front panel buttons
    TRISAbits.TRISA3 = 1; // POWER_BUTTON
    TRISAbits.TRISA4 = 1; // CDN_BUTTON
    TRISAbits.TRISA5 = 1; // LUP_BUTTON
    TRISBbits.TRISB2 = 1; // CUP_BUTTON
    TRISBbits.TRISB4 = 1; // FUNC_BUTTON
    TRISBbits.TRISB5 = 1; // LDN_BUTTON
    TRISBbits.TRISB6 = 1; // ANT_BUTTON
    TRISBbits.TRISB7 = 1; // TUNE_BUTTON

    // Front Panel bitbang SPI
    TRISAbits.TRISA6 = 0; // FP_CLOCK_PIN
    TRISCbits.TRISC4 = 0; // FP_STROBE_PIN
    TRISCbits.TRISC5 = 0; // FP_DATA_PIN

    // Front Panel LEDs
    TRISAbits.TRISA2 = 0; // POWER_LED_PIN
    TRISBbits.TRISB1 = 0; // ANT_LED_PIN
    TRISCbits.TRISC3 = 0; // BYPASS_LED_PIN

    // Relay SPI
    TRISCbits.TRISC0 = 0; // RELAY_CLOCK_PIN
    TRISCbits.TRISC1 = 0; // RELAY_DATA_PIN
    TRISCbits.TRISC2 = 0; // RELAY_STROBE_PIN

    // Meter port pins
    TRISCbits.TRISC6 = 0; // Meter TX
    TRISCbits.TRISC7 = 1; // Meter RX

    // Debug UART pins
    TRISDbits.TRISD2 = 0; // Debug TX
    TRISDbits.TRISD3 = 1; // Debug RX

    ANSELAbits.ANSELA0 = 1; // FWD_PIN
    ANSELAbits.ANSELA1 = 1; // REV_PIN

    // WPUAbits.WPUA3 = 1; // POWER_BUTTON
    WPUEbits.WPUE1 = 1; // POWER_BUTTON
    WPUAbits.WPUA4 = 1; // CDN_BUTTON
    WPUAbits.WPUA5 = 1; // LUP_BUTTON
    WPUBbits.WPUB2 = 1; // CUP_BUTTON
    WPUBbits.WPUB4 = 1; // FUNC_BUTTON
    WPUBbits.WPUB5 = 1; // LDN_BUTTON
    // WPUBbits.WPUB6 = 1; // ANT_BUTTON
    WPUEbits.WPUE2 = 1; // ANT_BUTTON
    WPUBbits.WPUB7 = 1; // TUNE_BUTTON
}