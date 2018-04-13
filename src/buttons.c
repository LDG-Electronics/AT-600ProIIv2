#include "includes.h"
#include "buttons.h"
#include "pins.h"
#include "timer.h"

/* ************************************************************************** */

// Bitmask used to debounce the button history
#define DEBOUNCE_MASK 0b11000111

// AT-600ProII has 8 front panel buttons
#define FRONT_PANEL_BUTTONS 8

// Button history storage
volatile uint8_t buttons[FRONT_PANEL_BUTTONS];

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

void buttons_init(void)
{
    // Initialize the button history array
    for (uint8_t i = 0; i < FRONT_PANEL_BUTTONS; i++)
    {
        buttons[i] = 0;
    }

    // Timer 6 configured using MPLABX MCC
    // Period is calculated to be exactly 5ms
    T6CLKbits.CS = 0b0010; // Clock source is FOSC
    T6CONbits.CKPS = 0b111; // Prescaler set to 1:128
    T6CONbits.OUTPS = 0b1001; // Postscaler set to 1:10

    PR6 = 0xF9; // Timer 6 period
    
    PIE9bits.TMR6IE = 1; // Enable Timer6 interrupt
    
    timer6_start();
}

void buttons_stop(void)
{
    timer6_stop();
}

/* -------------------------------------------------------------------------- */

/*  Notes on button_subsystem_ISR()

    This function is an Interrupt Vector Table compatible ISR to respond to the
    IRQ_TMR6 interrupt signal. This signal is generated whenever timer6
    overflows from 0xff to 0x00. The timer6 interrupt flag must be cleared by
    software.
    
    The ISR then checks the state of each button and updates the correct entry
    in the button history array. The first operation is a bit shift to slide
    the existing values over one, and drop the oldest value off the top end.
    Then the current state is read with the macro XXXX_BUTTON_PIN, and bitwise
    OR'd into the least significant bit.
*/
void __interrupt(irq(TMR6), high_priority) button_subsystem_ISR(void)
{
    timer6_IF_clear();

    // Grab current state of every button
    buttons[TUNE] <<= 1;
    buttons[TUNE] |= TUNE_BUTTON_PIN;
    
    buttons[FUNC] <<= 1;
    buttons[FUNC] |= FUNC_BUTTON_PIN;
    
    buttons[CUP] <<= 1;
    buttons[CUP] |= CUP_BUTTON_PIN;
    
    buttons[CDN] <<= 1;
    buttons[CDN] |= CDN_BUTTON_PIN;
    
    buttons[LUP] <<= 1;
    buttons[LUP] |= LUP_BUTTON_PIN;
    
    buttons[LDN] <<= 1;
    buttons[LDN] |= LDN_BUTTON_PIN;
    
    // TODO: ANT button is currently nonfunctional
    // buttons[ANT] <<= 1;
    // buttons[ANT] |= ANT_BUTTON_PIN;

    buttons[POWER] <<= 1;
    buttons[POWER] |= POWER_BUTTON_PIN;
}

/* -------------------------------------------------------------------------- */

// Returns 0 if no buttons are pressed
uint8_t get_buttons(void) // 2us
{
    // Grab the current state of all the button pins
    uint8_t buttons = ~PORTB & 0xf4;
    buttons = buttons | (!PORTAbits.RA3 << 3);
    buttons = buttons | (!PORTAbits.RA4 << 1);
    buttons = buttons | (!PORTAbits.RA5); 

    // TODO: ANT button is currently nonfunctional
    buttons = buttons & 0b10111111;

    return buttons;
}

// Returns 0 if no buttons are pressed
uint8_t get_buttons2(void) // 11us, will be faster after ANT buttons works
{
    uint16_t x = 0;
    
    for (uint8_t i = 0; i < FRONT_PANEL_BUTTONS; i++)
    {
        x += buttons[i];
    }

    x -= buttons[ANT]; // TODO: ANT button is currently nonfunctional

    return x;
}

/* -------------------------------------------------------------------------- */

// Returns 1 if rising edge is detected
uint8_t btn_is_pressed(buttonName_t buttonName)
{
    /*
        Mask out the middle three 'bouncy' bits, and compare the result with the
        'pressed' binary literal.  This evaluates true if the button was
        previously 'up', bounces in the middle, and has three continous polls of
        'down'.
        
        If the above conditions are met, we set the history to all down, so
        another call to this function won't falsely detect another rising edge. 
        Then, return 1 for true.
        
        If the conditions aren't met, return 0 for false. (Duh.)
    */
    
    if ((buttons[buttonName] & DEBOUNCE_MASK) == 0b00000111)
    {
        buttons[buttonName] = 0b11111111;
        return 1;
    }
    return 0;
}

// Returns 1 if falling edge is detected
uint8_t btn_is_released(buttonName_t buttonName)
{
    /*
        Mask out the middle three 'bouncy' bits, and compare the result with the
        'released' binary literal.  This evaluates true if the button was
        previously 'down', bounces in the middle, and has three continous polls of
        'up'.
        
        If the above conditions are met, we set the history to all up, so
        another call to this function won't falsely detect another falilng edge.
        Then, return 1 for true.
        
        If the conditions aren't met, return 0 for false. (Duh.)
    */
    
    if ((buttons[buttonName] & DEBOUNCE_MASK) == 0b11000000)
    {
        buttons[buttonName] = 0b00000000;
        return 1;
    }
    return 0;
}

// Returns 1 if buttons entire history is 'down', aka button is being held.
uint8_t btn_is_down(buttonName_t buttonName)
{
    return (buttons[buttonName] == 0b11111111);
}

// Returns 1 if buttons entire history is 'up', aka button is NOT held.
uint8_t btn_is_up(buttonName_t buttonName)
{
    return (buttons[buttonName] == 0b00000000);
}