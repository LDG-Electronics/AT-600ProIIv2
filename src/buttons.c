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

uint8_t get_buttons(void)
{
    uint8_t check = ~PORTB;

    check &= ~1; // mask off RB0, it's not a button

    // These two buttons are on RB6 and RB7, the two pins used by the ICD-U80
    // check &= ~(1 << 7); // mask off the tune button
    check &= ~(1 << 6); // mask off the antenna button
    
    return check;
}

// Only returns after all buttons have been released.
void wait_for_no_buttons(void)
{
    uint8_t i = 0;
    uint8_t x = 1;
    
    // TODO: Explain how this works.
    while (x != 0)
    {
        x = 0;
        for (i = 0; i < FRONT_PANEL_BUTTONS; i++)
        {
            x += buttons[i];
        }
    }
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