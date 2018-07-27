#include "hardware.h"
#include "display.h"
#include "events.h"
#include "flags.h"
#include "os/buttons.h"
#include "os/event_scheduler.h"
#include "os/log.h"
#include "os/shell/shell.h"
#include "os/system_time.h"
#include "peripherals/interrupt.h"
#include "peripherals/nonvolatile_memory.h"
#include "peripherals/oscillator.h"
#include "peripherals/pic18f46k42.h"
#include "peripherals/pins.h"
#include "peripherals/pps.h"
#include "peripherals/reset.h"
#include "relays.h"
#include "rf_sensor.h"
#include "shell_commands.h"
#include "tuning.h"

/* ************************************************************************** */
// Forward Declarations
void interrupt_init(void);

/* ************************************************************************** */

void startup(void) {
    // System setup
    oscillator_init();
    pins_init();
    interrupt_init();

    // OS setup
    shell_init();
    log_init();
    systick_init();
    event_scheduler_init();

    // Driver setup
    buttons_init();
    display_init();
    events_init();
    flags_init();
    nonvolatile_memory_init();
    relays_init();
    RF_sensor_init();
    stopwatch_init();
    tuning_init();

    // System setup, round 2
    register_all_shell_commands();

    pps_lock(); // PPS writes ABOVE THIS POINT ONLY

    reset_vector_handler();

    // Push out the initial relay settings
    put_relays(&currentRelays[system_flags.antenna]);

    // initialize the display
    play_animation_in_background(&right_crawl[0]);
    update_status_LEDs();
}

// TODO: rewrite the shutdown sequence
void shutdown(void) {
    // CPUDOZEbits.IDLEN = 0; // 0 = SLEEP, 1 = IDLE
    // IOCANbits.IOCAN3 = 1; // enable Interrupt-On-Change on the power button
    // PIE0bits.IOCIE = 1; // enable Interrupt-On-Change interrupt

    // asm("SLEEP");

    // PIE0bits.IOCIE = 0;
    // IOCANbits.IOCAN3 = 0;
    // IOCAF = 0;
}

/* -------------------------------------------------------------------------- */
// ISRs

// Wake-from-shutdown ISR
void __interrupt(irq(IOC), high_priority) IOC_ISR(void) {
    // interrupt on change for pin IOCAF3
    if (IOCAFbits.IOCAF3 == 1) {
        IOCAFbits.IOCAF3 = 0;
        PIE0bits.IOCIE = 0;
        IOCANbits.IOCAN3 = 0;
    }
}