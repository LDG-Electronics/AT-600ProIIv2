#ifndef _SYSTEM_TICK_H_
#define _SYSTEM_TICK_H_

/* ************************************************************************** */
/*  Notes on using the System Tick module

    This module counts the system's uptime in milliseconds. It functions
    similarly to the millis() function from the Arduino platform.

    There are several differences between millis() and systick_read():
    
    millis() stores its count in a uint32_t, meaning it takes roughly 49 days
    for millis() to overflow.
    systick_read() stores its count in a uint24_t, meaning it takes roughly
    4.6 hours to overflow.

    millis() requires an interrupt to fire every millisecond, and an ISR that
    increments a 4-byte-long uint32_t. This is a very quick ISR, but it's less
    than ideal that this ISR will need to be running all the time, behind any
    other operations you intend to do on your system.
    systick_read() uses no interrupt, so it still counts time during critical
    sections of code and will never contribute to interrupt-based timer bugs.

    This module uses several hardware features of the K42. One hardware timer
    of any variety needs to be configured to overflow every millisecond.

    This timer is then configured as the clock source of the Signal Measurement
    Timer, a special 24-bit counter. This causes the SMT to increment once every
    millisecond, completely in the background. No interrupts are necessary to
    count time.

    The SMT has another special feature that assists with read operations.
    Manipulating a 24-bit value on an 8-bit system takes several operations,
    potentially causing data corruption if the register value changes in the
    middle of a read. The SMT1STAT register has a bit that causes the SMT count
    register to be copied to a buffer, which can then be read from without
    worrying about atomic access.

    Maximum countable time:
    UINT24_MAX = 16,777,216 ticks until overflow

    Each tick = 1 millisecond:
    16,777,216 / 1000 = 16,777.216 seconds
    16,777 / 60 = 279.6 minutes
    279 minutes / 60 = 4.6 hours until overflow
    

*/
/* ************************************************************************** */

typedef uint24_t system_time_t;

/* ************************************************************************** */

extern void systick_init(void);

/* -------------------------------------------------------------------------- */

extern system_time_t systick_read(void);

extern system_time_t systick_elapsed_time(system_time_t startTime);

extern void systick_delay(uint16_t mseconds);

#endif