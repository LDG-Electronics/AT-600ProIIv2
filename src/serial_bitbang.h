#ifndef SERIAL_BITBANG_H
#define SERIAL_BITBANG_H

/* ************************************************************************** */

/*  Bitbang serial module for debug purposes.
    
    Currently tested and stable at up to 115200 baud on a 32Mhz clock. 
    
    Resources required by this module:
        No interrupts
        One GPIO
        One timer with a period or compare register and auto-rollover.
            On PIC18F2X/4XK22, timers 2/4/6 qualify.
            Timer should be left running, with interrupts disabled.
            Timer Interrupt Flag bit is used to clock the serial out.
        
    This module is currently configured for Timer6 on a PIC18F45k22.
    
    115200 baud = 8.68 us per bit
    32mhz/4 = 8mhz instruction clock
    8mhz instruction clock = 125 ns per instruction
    125 ns x 69 = 8.625 us

    Settings:
    Prescaler = 1:1
    Postscaler = 1:1
    TMR4 Preload = 69
    Actual Interrupt Time = 8.625 us
    No interrupt
    Should be left running

    NOTE: Timer6 should be started during setup and left running.
          Bitbang serial works better with a steady tempo.
          This prevents multiple sequential bytes from bunching and keeps the
          output from being jittery.
*/

// Setup
extern void serial_bitbang_init(void);

// Serial Bitbang transmit functions
extern void serial_tx_char(const char);
extern void serial_tx_string(const char *string);

#endif
