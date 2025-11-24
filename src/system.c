#include "display.h"
#include "flags.h"
#include "os/buttons.h"
#include "os/judi/judi.h"
#include "os/logging.h"
#include "os/shell/shell.h"
#include "os/shell/shell_command_processor.h"
#include "os/stopwatch.h"
#include "os/system_time.h"
#include "peripherals/device_information.h"
#include "peripherals/interrupt.h"
#include "peripherals/oscillator.h"
#include "peripherals/pic_header.h"
#include "peripherals/ports.h"
#include "peripherals/pps.h"
#include "peripherals/reset.h"
#include "peripherals/timer.h"
#include "peripherals/uart.h"
#include "pins.h"
#include "relays.h"
#include "rf_sensor.h"
#include "tuning.h"
#include "usb/messages.h"

/* ************************************************************************** */

// Set up the timer for the button isr
static void button_isr_init(void) {
    // Timer 6 configured using MPLABX MCC
    // Period is calculated to be exactly 5ms
    timer6_clock_source(TMR2_CLK_FOSC);
    timer6_prescale(TMR_PRESCALE_128);
    timer6_postscale(TMR_POSTSCALE_10);
    timer6_period_set(0xF9);
    timer6_interrupt_enable();
    timer6_start();
}

// call scan_buttons() every 5ms
void __interrupt(irq(TMR6), high_priority) button_ISR(void) {
    timer6_IF_clear();

    scan_buttons();
}

/* ************************************************************************** */

static void system_init(void) {
    internal_oscillator_init();
    interrupt_init();
    port_init();
    pins_init();
    device_information_init();
}

#ifdef DEVELOPMENT
extern void sh_adc(int argc, char **argv);
extern void sh_bar(int argc, char **argv);
extern void sh_eeprom(int argc, char **argv);
extern void sh_flash(int argc, char **argv);
extern void sh_memory(int argc, char **argv);
extern void sh_poly(int argc, char **argv);
extern void sh_relays(int argc, char **argv);
extern void sh_rfmon(int argc, char **argv);
extern void sh_romedit(int argc, char **argv);
extern void sh_tune(int argc, char **argv);
extern void sh_usb(int argc, char **argv);
#endif

static void OS_init(void) {
#ifdef DEVELOPMENT
    uart_config_t config = UART_get_config(2);
    config.baud = _1000000;
    config.txPin = PPS_DEBUG_TX_PIN;
    config.rxPin = PPS_DEBUG_RX_PIN;
    create_uart_buffers(debug, config, 128);
    serial_port_init(&config);

    shell_init();

    shell_register_command(sh_adc, "adc");
    shell_register_command(sh_bar, "bar");
    shell_register_command(sh_eeprom, "eeprom");
    shell_register_command(sh_flash, "flash");
    shell_register_command(sh_memory, "memory");
    shell_register_command(sh_poly, "poly");
    shell_register_command(sh_relays, "relays");
    shell_register_command(sh_rfmon, "rfmon");
    shell_register_command(sh_romedit, "romedit");
    shell_register_command(sh_tune, "tune");
    shell_register_command(sh_usb, "usb");
#endif

    buttons_init(NUMBER_OF_BUTTONS, buttonFunctions);
    button_isr_init();

    logging_init();
    system_time_init();
    stopwatch_init();
}

static void application_init(void) {
    flags_init();
    relays_init();
    display_init();
    RF_sensor_init();
    tuning_init();

#if defined DEVELOPMENT && defined USB_ENABLED
    uart_config_t config = UART_get_config(1);
    config.baud = _1000000;
    config.txPin = PPS_USB_TX_PIN;
    config.rxPin = PPS_USB_RX_PIN;
    create_uart_buffers(debug, config, 128);
    usb_port_init(&config);

    // usb_println("USB initialized");

    judi_init(respond);
#endif
}

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