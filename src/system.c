#include "hardware.h"
#include "display.h"
#include "flags.h"
#include "memory.h"
#include "os/buttons.h"
#include "os/log.h"
#include "os/shell/shell.h"
#include "os/stopwatch.h"
#include "os/system_time.h"
#include "peripherals/device_information.h"
#include "peripherals/interrupt.h"
#include "peripherals/oscillator.h"
#include "peripherals/pic_header.h"
#include "peripherals/ports.h"
#include "peripherals/pps.h"
#include "peripherals/reset.h"
#include "peripherals/uart.h"
#include "pins.h"
#include "relays.h"
#include "rf_sensor.h"
#include "tuning.h"
#include "usb/usb.h"

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

    TODO: write down the init ordering rules that live in daelon's head
*/

static void system_init(void) {
    internal_oscillator_init();
    interrupt_init();
    port_init();
    pins_init();
    device_information_init();
}

static void OS_init(void) {
    uart_config_t config = UART_get_config(2);
    config.baud = _115200;
    config.txPin = PPS_DEBUG_TX_PIN;
    config.rxPin = PPS_DEBUG_RX_PIN;
    shell_init(UART_init(config));

    buttons_init();

    log_init();
    system_time_init();
    stopwatch_init();
}

static void application_init(void) {
    flags_init();
    relays_init();
    display_init();
    RF_sensor_init();
    tuning_init();
    memory_init();

    uart_config_t config = UART_get_config(1);
    config.baud = _115200;
    config.txPin = PPS_USB_TX_PIN;
    config.rxPin = PPS_USB_RX_PIN;
    usb_init(UART_init(config));
}

/* ************************************************************************** */

void startup(void) {
    system_init();
    OS_init();

    check_hardware_reset_flags(); // should be after OS_init(), so logging is on

    application_init();

    pps_lock(); // PPS writes ABOVE THIS POINT ONLY

    // Attempt to load previously saved flags
    load_flags();

    // only do things if the power is on
    if (systemFlags.powerStatus == 1) {
        // Push out the initial relay settings
        if (put_relays(currentRelays[systemFlags.antenna]) == -1) {
            // TODO: what do we do on relayerror?
        }

        // initialize the display
        update_status_LEDs();
        play_animation(&right_crawl[0]);
    } else {
        if (put_relays(bypassRelays) == -1) {
            // TODO: what do we do on relayerror?
        }
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