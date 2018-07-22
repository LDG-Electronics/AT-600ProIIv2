#include "../includes.h"

#include "../peripherals/pins.h"
#include "../peripherals/timer.h"
#include "buttons.h"
#include <stdarg.h>

/* ************************************************************************** */

// Bitmask used to debounce the button history
#define DEBOUNCE_MASK 0b11000111

// Button history storage
volatile uint8_t buttonHistory[NUMBER_OF_BUTTONS];

/* ************************************************************************** */

/*  These button checker functions have been derived from Eliot Williams
    'Ultimate Debouncer', found at Hackaday.

    The general idea is to use an interrupt routine that polls the physical
    buttons at a particular rate.  A history of button state is stored in a
    variable by bit shifting left one and setting the least-significant bit
    to the polled bit.

    These checker functions compare the button history against a bit mask to
    simultaneously debounce and determine one of 5 different states.
*/

/* ************************************************************************** */

void buttons_init(void) {
    // Initialize the button history array
    for (uint8_t i = 0; i < NUMBER_OF_BUTTONS; i++) {
        buttonHistory[i] = 0;
    }

    // Timer 6 configured using MPLABX MCC
    // Period is calculated to be exactly 5ms
    timer6_clock_source(TMR_CLK_FOSC);
    timer6_prescale(TMR_PRE_1_128);
    timer6_postscale(TMR_POST_1_10);

    timer6_period_set(0xF9);

    timer6_interrupt_enable();

    timer6_start();
}

/* -------------------------------------------------------------------------- */

/*  Notes on button_subsystem_ISR()

    This function is an Interrupt Vector Table compatible ISR to respond to the
    TMR6 interrupt signal. This signal is generated whenever timer6 overflows
    from 0xff to 0x00. The timer6 interrupt flag must be cleared by software.

    The ISR then checks the state of each button and updates the correct entry
    in the button history array. The first operation is a bit shift to slide the
    existing values over one, and drop the oldest value off the top end. Then
    the current state is read with the macro XXXX_BUTTON_PIN, and bitwise OR'd
    into the least significant bit.
*/

// CHECK_BUTTON() is used to expand BUTTON_LIST and generate the button ISR
#define CHECK_BUTTON(NAME)                                                     \
    do {                                                                       \
        buttonHistory[NAME] <<= 1;                                             \
        buttonHistory[NAME] |= BUTTON_PIN_NAME_FORMAT(NAME);                   \
    } while (0);

void __interrupt(irq(TMR6), high_priority) button_subsystem_ISR(void) {
    timer6_IF_clear();

    // X Macro, expands BUTTON_LIST using CHECK_BUTTON(), defined above
#define X(value) CHECK_BUTTON(value)
    BUTTON_LIST
#undef X
}

/* -------------------------------------------------------------------------- */

// Returns 1 if any button is currently down
uint8_t get_buttons(void) {
    for (uint8_t i = 0; i < NUMBER_OF_BUTTONS; i++) {
        if (btn_is_down(i))
            return 1;
    }

    return 0;
}

/* -------------------------------------------------------------------------- */

// Returns a 1 if any of the listed buttons is experiencing the specified state
uint8_t check_multiple_buttons(button_check_t btn_is_xxx,
                               uint8_t numberOfButtons, ...) {
    va_list ap;

    va_start(ap, numberOfButtons);

    for (uint8_t i = 0; i < numberOfButtons; i++) {
        if (btn_is_xxx(va_arg(ap, buttonName_t))) {
            va_end(list);
            return 1;
        }
    }

    va_end(list);
    return 0;
}

/* -------------------------------------------------------------------------- */

// Returns 1 if buttons entire history is 'down', aka button is being held.
uint8_t btn_is_down(buttonName_t buttonName) {
    return (buttonHistory[buttonName] == 0b11111111);
}

// Returns 1 if buttons entire history is 'up', aka button is NOT held.
uint8_t btn_is_up(buttonName_t buttonName) {
    return (buttonHistory[buttonName] == 0b00000000);
}

/*  Mask out the middle three 'bouncy' bits, and compare the result with the
    'pressed' binary literal.  This evaluates true if the button was
    previously 'up', bounces in the middle, and has three continuous polls
    of 'down'.

    If the above conditions are met, we set the history to all down, so
    another call to this function won't falsely detect another rising edge.
    Then, return 1 for true.

    If the conditions aren't met, return 0 for false. (Duh.)
*/
// Returns 1 if rising edge is detected
uint8_t btn_is_pressed(buttonName_t buttonName) {
    if ((buttonHistory[buttonName] & DEBOUNCE_MASK) == 0b00000111) {
        buttonHistory[buttonName] = 0b11111111;
        return 1;
    }
    return 0;
}

/*  Mask out the middle three 'bouncy' bits, and compare the result with the
    'released' binary literal.  This evaluates true if the button was
    previously 'down', bounces in the middle, and has three continuous polls
    of 'up'.

    If the above conditions are met, we set the history to all up, so
    another call to this function won't falsely detect another falling edge.
    Then, return 1 for true.

    If the conditions aren't met, return 0 for false. (Duh.)
*/
// Returns 1 if falling edge is detected
uint8_t btn_is_released(buttonName_t buttonName) {
    if ((buttonHistory[buttonName] & DEBOUNCE_MASK) == 0b11000000) {
        buttonHistory[buttonName] = 0b00000000;
        return 1;
    }
    return 0;
}
