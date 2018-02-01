#include "includes.h"
#include "drivers/mcc.h"

/* ************************************************************************** */

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
    TRISD = 0xFF;
    TRISE = 0xFF;
    TRISF = 0xFF;

    // Front Panel bitbang SPI - AT-100ProII
    TRISCbits.TRISC4 = 0;
    TRISCbits.TRISC5 = 0;
    TRISCbits.TRISC1 = 0;
    
    // Front Panel LEDs - AT-100ProII
    TRISBbits.TRISB5 = 0;
    TRISBbits.TRISB4 = 0;

    // LED strip control pins
    TRISCbits.TRISC0 = 0;
    TRISCbits.TRISC1 = 0;

    // 
    TRISCbits.TRISC7 = 0;

    // Z-11ProII LED Pins
    TRISBbits.TRISB5 = 0; // LED A
    TRISAbits.TRISA7 = 0; // LED B
    TRISBbits.TRISB4 = 0; // LED C
    TRISCbits.TRISC1 = 0; // LED D

    // Z-11 Frequency Counter pin
    TRISEbits.TRISE0 = 1;

    TRISFbits.TRISF6 = 0;

    // Controls - YT-1200
    TRISAbits.TRISA3 = 0; // LED, output
    TRISBbits.TRISB0 = 1; // BUTTON, input
  
    // Output latch - explicitly drive all outputs low
    LATA = 0;    
    LATB = 0;    
    LATC = 0; 
    LATD = 0;    
    LATE = 0;    
    LATF = 0;    
    
    // Analog Select; 0 = analog mode is disabled, 1 = analog mode is enabled
    ANSELA = 0;
    ANSELB = 0;
    ANSELC = 0;
    ANSELD = 0;
    ANSELE = 0;
    ANSELF = 0;

    // Weak Pull-up; 0 = pull-up disabled, 1 = pull-up enabled
    WPUA = 0xff;
    WPUB = 0;
    WPUC = 0;
    WPUD = 0;
    WPUE = 0;
    WPUF = 0;

    // Open-Drain Control; 0 = Output drives both high and low, 1 = 
    ODCONA = 0;
    ODCONB = 0;
    ODCONC = 0;
    ODCOND = 0;
    ODCONE = 0;
    ODCONF = 0;

    // Slew Rate Control; 0 = maximum slew rate, 1 = limited slew rate
    SLRCONA = 0;
    SLRCONB = 0;
    SLRCONC = 0;
    SLRCOND = 0;
    SLRCONE = 0;
    SLRCONF = 0;

    // Input (Logic) Level Control; 0 = TTL, 1 = Schmitt Trigger
    INLVLA = 0xff;
    INLVLB = 0xff;
    INLVLC = 0xff;
    INLVLD = 0xff;
    INLVLE = 0xff;
    INLVLF = 0xff;
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

    T1CKIPPS = 0x20;

    PPSLOCK = 0x55;
    PPSLOCK = 0xAA;
    PPSLOCKbits.PPSLOCKED = 0x01; // lock PPS
}

/* -------------------------------------------------------------------------- */



void startup(void)
{
    POWER_HOLD = 1;

    SYSTEM_Initialize();
    osc_init();
    port_init();
    pps_init();
    interrupt_init();
    
    // Driver setup
    delay_init();
    buttons_init();
    spi_init();
    display_init();
    serial_bitbang_init();
    RF_sensor_init();
}

void shutdown(void)
{
    asm("SLEEP");
}