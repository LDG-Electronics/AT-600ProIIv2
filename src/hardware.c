#include "hardware.h"
#include "display.h"
#include "flags.h"
#include "memory.h"
#include "os/buttons.h"
#include "os/log.h"
#include "os/shell/shell.h"
#include "os/stopwatch.h"
#include "os/system_time.h"
#include "peripherals/device_header.h"
#include "peripherals/interrupt.h"
#include "peripherals/nonvolatile_memory.h"
#include "peripherals/oscillator.h"
#include "peripherals/pins.h"
#include "peripherals/pps.h"
#include "peripherals/reset.h"
#include "relays.h"
#include "rf_sensor.h"
#include "shell_commands.h"
#include "tuning.h"

/* ************************************************************************** */
/*  Notes on startup()

    This function shall be called exactly once: In main(), before any main loop
    or task scheduler is started.

    The only contents of startup() should be the various xxx_init() functions
    used to initialize various parts of the system.

    Facts about the init process:
    init functions may or may not have dependecies
    init functions may or may not be idempotent
    init functions for peripherals should be called during the init for whatever
    driver or system uses that peripheral

*/
void startup(void) {
    // System setup
    oscillator_init();
    pins_init();
    interrupt_init();

    // OS setup
    shell_init((PPS_PORT_D & PPS_PIN_3), &RD2PPS);
    log_init();
    system_time_init();

    // Driver setup
    relays_init();
    buttons_init();
    display_init();
    RF_sensor_init();
    stopwatch_init();
    tuning_init();
    memory_init();
    flags_init();

    pps_lock(); // PPS writes ABOVE THIS POINT ONLY

    reset_vector_handler();

    // Attempt to load previously saved flags
    load_flags();

    // if the unit is off, make sure that we're quiet
    if (systemFlags.powerStatus == 1) {
        // Push out the initial relay settings
        put_relays(&currentRelays[systemFlags.antenna]);

        // initialize the display
        update_status_LEDs();
        play_animation(&right_crawl[0]);
    } else {
        put_relays(&bypassRelays);
    }
}

/* -------------------------------------------------------------------------- */

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

// Wake-from-shutdown ISR
void __interrupt(irq(IOC), high_priority) IOC_ISR(void) {
    // interrupt on change for pin IOCAF3
    if (IOCAFbits.IOCAF3 == 1) {
        IOCAFbits.IOCAF3 = 0;
        PIE0bits.IOCIE = 0;
        IOCANbits.IOCAN3 = 0;
    }
}