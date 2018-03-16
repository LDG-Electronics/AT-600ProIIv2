#include "includes.h"

/* ************************************************************************** */
// Forward Declarations
void osc_init(void);
void port_init(void);
void pps_init(void);

/* ************************************************************************** */

void startup(void)
{
    osc_init();
    port_init();
    pps_init();
    interrupt_init();
    
    // Driver setup
    delay_init();
    adc_init();
    buttons_init();
    spi_init();
    display_init();
    serial_bitbang_init();
    RF_sensor_init();
    relays_init();

    // Recall previous stuff from memory
    load_flags();
    if (system_flags.inBypass == 1) put_relays(&bypassRelays);
    if (system_flags.inBypass == 0) put_relays(&currentRelays);

    // This delay is needed during dev because the TUNE button is shared with a
    // programming pin
    // TODO: reassess this during verification
    delay_ms(250);

    // initialize the display
    play_interruptable_animation(&right_crawl);
    update_antenna_led();
    update_bypass_led();
    update_power_led();

    print_format(BRIGHT, RED);
    print_str_ln("Hello!");
}

void shutdown(void)
{
    //TODO: implement sleep mode/power saving features
    asm("SLEEP");
}

/* -------------------------------------------------------------------------- */

void osc_init(void)
{
    // NOSC HFINTOSC; NDIV 1; 
    OSCCON1 = 0x60;
    // CSWHOLD may proceed; SOSCPWR Low power; 
    OSCCON3 = 0x00;
    // MFOEN disabled; LFOEN disabled; ADOEN disabled; SOSCEN disabled; EXTOEN disabled; HFOEN disabled; 
    OSCEN = 0x00;
    // HFFRQ 64_MHz; 
    OSCFRQ = 0x08;
    // TUN 0; 
    OSCTUNE = 0x00;
}

void port_init(void)
{
    // Tri-state control; 0 = output enabled, 1 = output disabled
    // Explicitly disable all outputs
    TRISA = 0xFF;
    TRISB = 0xFF;
    TRISC = 0xFF;

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
    TRISAbits.TRISA2 = 0; // POWER_LED
    TRISBbits.TRISB1 = 0; // ANT_LED
    TRISCbits.TRISC3 = 0; // BYPASS_LED

    // Relay SPI
    TRISCbits.TRISC0 = 0; // RELAY_CLOCK_PIN
    TRISCbits.TRISC1 = 0; // RELAY_DATA_PIN
    TRISCbits.TRISC2 = 0; // RELAY_STROBE_PIN

    // Meter port pins
    TRISCbits.TRISC6 = 0; // Meter TX
    TRISCbits.TRISC7 = 1; // Meter RX

    // Output latch - explicitly drive all outputs low
    LATA = 0;    
    LATB = 0;    
    LATC = 0;   
    
    // Analog Select; 0 = analog mode is disabled, 1 = analog mode is enabled
    ANSELA = 0;
    ANSELB = 0;
    ANSELC = 0;

    ANSELAbits.ANSELA0 = 1; // FWD_PIN
    ANSELAbits.ANSELA1 = 1; // REV_PIN

    // Weak Pull-up; 0 = pull-up disabled, 1 = pull-up enabled
    WPUA = 0;
    WPUB = 0;
    WPUC = 0;

    WPUAbits.WPUA3 = 1; // POWER_BUTTON
    WPUAbits.WPUA4 = 1; // CDN_BUTTON
    WPUAbits.WPUA5 = 1; // LUP_BUTTON
    WPUBbits.WPUB2 = 1; // CUP_BUTTON
    WPUBbits.WPUB4 = 1; // FUNC_BUTTON
    WPUBbits.WPUB5 = 1; // LDN_BUTTON
    WPUBbits.WPUB6 = 1; // ANT_BUTTON
    WPUBbits.WPUB7 = 1; // TUNE_BUTTON

    // Open-Drain Control; 0 = Output drives both high and low, 1 = 
    ODCONA = 0;
    ODCONB = 0;
    ODCONC = 0;

    // Slew Rate Control; 0 = maximum slew rate, 1 = limited slew rate
    SLRCONA = 0;
    SLRCONB = 0;
    SLRCONC = 0;

    // Input (Logic) Level Control; 0 = TTL, 1 = Schmitt Trigger
    INLVLA = 0xff;
    INLVLB = 0xff;
    INLVLC = 0xff;
}

void pps_init(void)
{
    PPSLOCK = 0x55;
    PPSLOCK = 0xAA;
    PPSLOCKbits.PPSLOCKED = 0x00; // unlock PPS

    // RC0PPS = 0x1E;   //RC0->SPI1:SCK1;
    // SPI1SCKPPSbits.SPI1SCKPPS = 0x10;   //RC0->SPI1:SCK1;
    // SPI1SDIPPSbits.SPI1SDIPPS = 0x12;   //RC2->SPI1:SDI1;
    // RC1PPS = 0x1F;   //RC1->SPI1:SDO1;

    T3CKIPPS = 0b00001000;

    PPSLOCK = 0x55;
    PPSLOCK = 0xAA;
    PPSLOCKbits.PPSLOCKED = 0x01; // lock PPS
}