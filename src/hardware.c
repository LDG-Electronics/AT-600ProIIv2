#include "includes.h"

/* ************************************************************************** */
// Forward Declarations
void interrupt_init(void);
void check_reset_vector(void);

/* ************************************************************************** */

void startup(void)
{
    // System setup
    oscillator_init();
    pins_init();
    interrupt_init();
    shell_init();
    systick_init();
    event_scheduler_init();

    // Driver setup
    buttons_init();
    display_init();
    events_init();
    flags_init();
    frequency_counter_init();
    relays_init();
    RF_sensor_init();
    shell_commands_init();
    stopwatch_init();
    
    pps_lock(); // PPS writes ABOVE THIS POINT ONLY

    check_reset_vector();

    // Push out the initial relay settings
    put_relays(&currentRelays[system_flags.antenna]); // must be after flags_init()

    // initialize the display
    play_animation_in_background(&right_crawl);
    update_status_LEDs(); // must be after flags_init()
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