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
    timer5_init();
    
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