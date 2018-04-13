#include "includes.h"
#include "pps.h"

/* ************************************************************************** */
// Forward Declarations
void osc_init(void);
void port_init(void);

/* ************************************************************************** */

void startup(void)
{
    // System setup
    osc_init();
    port_init();
    interrupt_init();
    systick_init();

    // Driver setup
    buttons_init();
    display_init();
    flags_init();
    log_init();
    meter_init();
    relays_init();
    RF_sensor_init();
    stopwatch_init();
    
    pps_lock(); // PPS writes ABOVE THIS POINT ONLY

    // Push out the initial relay settings
    put_relays(&currentRelays[system_flags.antenna]);

    // initialize the display
    play_animation(&right_crawl);
    display_clear();
    update_antenna_led();
    update_bypass_led();
    update_power_led();

    #if LOG_LEVEL_SYSTEM > LOG_SILENT
    // Debug greeting
    print_ln();
    print_ln();
    print_format(BRIGHT, RED);
    print_str_ln("Hello!");
    #endif
}

void shutdown(void)
{
    CPUDOZEbits.IDLEN = 0; // 0 = SLEEP, 1 = IDLE
    IOCANbits.IOCAN3 = 1; // enable Interrupt-On-Change on the power button
    PIE0bits.IOCIE = 1; // enable Interrupt-On-Change interrupt
    timer5_stop();

    asm("SLEEP");

    PIE0bits.IOCIE = 0;
    IOCANbits.IOCAN3 = 0;
    IOCAF = 0;

    timer5_start();
}

/* -------------------------------------------------------------------------- */

void osc_init(void)
{
    OSCCON1 = 0x60; // NOSC HFINTOSC; NDIV 1;
    OSCCON3 = 0x00; // CSWHOLD may proceed; SOSCPWR Low power;
    OSCEN = 0x00; // MFOEN disabled; LFOEN disabled; ADOEN disabled; 
                  // SOSCEN disabled; EXTOEN disabled; HFOEN disabled;
    OSCFRQ = 0x08; // HFFRQ 64_MHz;
    OSCTUNE = 0x00; // TUN 0;
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