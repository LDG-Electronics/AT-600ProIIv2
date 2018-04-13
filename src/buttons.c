#include "includes.h"

/* ************************************************************************** */

#define MASK 0b11000111

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
    // Timer5 setup
    T5CLK = 1; // Select Fosc/4 as clock source
    T5CONbits.CKPS = 0b01; // 1:2 prescale
    T5CONbits.RD16 = 1; // 16 bit mode, for atomic operation
    
    PIE8bits.TMR5IE = 1; // Enable Timer5 interrupt
    
    timer5_start();
}

/* -------------------------------------------------------------------------- */

/*  This is an example of the subscriber half of the button processing subsystem_flags.
    
    This example is pulled from the interrupt handler in the AT-100ProII software,
    although it is not necessary to use the Interrupt Service Routine(ISR).
    
    The AT-100ProII has six front panel buttons:
        Tune
        Func
        L Up
        L Dn
        C Up
        C Dn
        ANT
    
    This interrupt is triggered by Timer 5, which is set to overflow every 5ms.
    The ISR stops the timer, clears the timer's interrupt flag, and reloads the
    correct values to Timer 5's registers.
    
    The next block checks the state of each button and updates the correct entry
    in the button history array.  The first operation is a bit shift to slide
    the existing values over one, and drop the oldest value off the top end.
    Then the current state is read with the macro XXXX_BUTTON, and bitwise OR'd
    into the least significant bit.
    
    Timer 5 is then enabled.
*/
void __interrupt(irq(TMR5), high_priority) TMR5_ISR(void)
{
    timer5_stop();
    timer5_IF_clear();
    TMR5H = 0x63;   // reset timer
    TMR5L = 0xC0;

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
    
    // buttons[ANT] <<= 1; //! ANT button is disabled
    // buttons[ANT] |= ANT_BUTTON_PIN;

    buttons[POWER] <<= 1;
    buttons[POWER] |= POWER_BUTTON_PIN;
    
    timer5_start();
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
    
    if ((buttons[buttonName] & MASK) == 0b00000111)
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
    
    if ((buttons[buttonName] & MASK) == 0b11000000)
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