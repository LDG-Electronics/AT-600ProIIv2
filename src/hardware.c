#include "includes.h"

/* ************************************************************************** */
// Forward Declarations
void osc_init(void);
void port_init(void);
void interrupt_init(void);
void check_reset_vector(void);

/* ************************************************************************** */

void startup(void)
{
    // System setup
    osc_init();
    port_init();
    pins_init();
    interrupt_init();
    shell_init();
    systick_init();
    task_manager_init();

    // Driver setup
    buttons_init();
    display_init();
    flags_init();
    relays_init();
    RF_sensor_init();
    stopwatch_init();
    
    pps_lock(); // PPS writes ABOVE THIS POINT ONLY

    check_reset_vector();

    // Push out the initial relay settings
    put_relays(&currentRelays[system_flags.antenna]); // must be after flags_init()

    // initialize the display
    begin_background_animation(&right_crawl);
    // play_animation(&right_crawl);
    // display_clear();
    update_status_LEDs(); // must be after flags_init()

    // 
    task_register("SWR_Monitor", check_SWR_and_update_meters, 100, 250);
}

void shutdown(void)
{
    CPUDOZEbits.IDLEN = 0; // 0 = SLEEP, 1 = IDLE
    IOCANbits.IOCAN3 = 1; // enable Interrupt-On-Change on the power button
    PIE0bits.IOCIE = 1; // enable Interrupt-On-Change interrupt
    buttons_stop();

    asm("SLEEP");

    PIE0bits.IOCIE = 0;
    IOCANbits.IOCAN3 = 0;
    IOCAF = 0;

    buttons_init();
}

void __interrupt(irq(IRQ_IOC), high_priority) IOC_ISR(void)
{   
    // interrupt on change for pin IOCAF3
    if(IOCAFbits.IOCAF3 == 1)
    {
        IOCAFbits.IOCAF3 = 0;
        PIE0bits.IOCIE = 0;
        IOCANbits.IOCAN3 = 0;
    }
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
    TRISD = 0xFF;
    TRISE = 0xFF;

    // Output latch - explicitly drive all outputs low
    LATA = 0;    
    LATB = 0;    
    LATC = 0;   
    LATC = 0;   
    LATC = 0;   
    
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

void interrupt_init(void)
{
    INTCON0bits.IPEN = 0; // Disable priority levels on interrupts
    
    // Clear all peripheral interrupts
    PIE1 = 0x0;
    PIE2 = 0x0;
    PIE3 = 0x0;
    PIE4 = 0x0;
    PIE5 = 0x0;
    PIE6 = 0x0;
    PIE7 = 0x0;
    PIE8 = 0x0;
    PIE9 = 0x0;
    
    INTCON0bits.GIE = 1;
}

void check_reset_vector(void)
{
    if(PCON0bits.STKOVF == 1) {
        println("");
        println("");
        println(">>> Stack Overflow <<<");
        while(1); // trap
    }
    
    if(PCON0bits.STKUNF == 1) {
        println("");
        println("");
        println(">>> Stack Underflow <<<");
        while(1); // trap
    }
    
    // if(PCON0bits.RMCLR == 0) {
    //     println("");
    //     println("");
    //     println(">>> MCLR RESET <<<");
    //     while(1); // trap
    // }
    
    // if(PCON0bits.RI == 0) {
    //     println("");
    //     println("");
    //     println(">>> RESET <<<");
    //     while(1); // trap
    // }
    
}