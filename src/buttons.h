#ifndef _BUTTONS_H_
#define _BUTTONS_H_

/* ************************************************************************** */

/*  Notes on using the button processing subsystem_flags.
    
    This module is designed as a publisher/subscriber model, with two loosely
    coupled components.
    
    The publisher polls the physical buttons at a regular interval, 5ms is
    a typical value.  The publisher then records the current state of each
    button to a ledger or history.  This works best when it happens 
    asynchronously with the subscriber half of the module, such as in an 
    interrupt service routine triggered by a timer overflow, or a dedicated
    RTOS thread.
    
    The subscriber half is applied wherever it is desired to check on the status
    of an external input.  A call to one of the btn_is_xxxx() functions will
    look at the history that's been recorded by the publisher and return a yes
    or no, based on whether the history matches a predetermined pattern.  The
    specific pattern is different for each that is being evaluated, but they all
    provide some level of software debouncing with hysteresis.
*/

/* ************************************************************************** */

#define FRONT_PANEL_BUTTONS 8

// This enum should contain one entry for every front panel button.
typedef enum {TUNE, FUNC, CUP, CDN, LUP, LDN, ANT, POWER} buttonName_t;

// This is only public so it can be accessed by the ISR.  If not ISR, no touchy.
extern volatile uint8_t buttons[FRONT_PANEL_BUTTONS];

/* ************************************************************************** */

// Setup
extern void buttons_init(void);

// Deprecated
extern uint8_t get_buttons(void);
extern uint8_t get_buttons2(void);

// button handley-ness
extern uint8_t btn_is_pressed(buttonName_t buttonName);
extern uint8_t btn_is_released(buttonName_t buttonName);
extern uint8_t btn_is_down(buttonName_t buttonName);
extern uint8_t btn_is_up(buttonName_t buttonName);

#endif

/* ************************************************************************** */

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


# if 0 // <- do not use, this is example code.
/*
    void interrupt_high(void) __interrupt 1
    {
        // 5ms tick for button polling
        if (TIMER5_IF == 1)
        {
            TIMER5_ON = 0;  // stop timer
            TIMER5_IF = 0;  // clear flag
            TMR5H = 0x63;   // reset timer
            TMR5L = 0xC0;

            // Grab current state of every button
            buttons[TUNE] <<= 1;
            buttons[TUNE] |= TUNE_BUTTON;
            
            buttons[FUNC] <<= 1;
            buttons[FUNC] |= FUNC_BUTTON;
            
            buttons[CUP] <<= 1;
            buttons[CUP] |= CUP_BUTTON;
            
            buttons[CDN] <<= 1;
            buttons[CDN] |= CDN_BUTTON;
            
            buttons[LUP] <<= 1;
            buttons[LUP] |= LUP_BUTTON;
            
            buttons[LDN] <<= 1;
            buttons[LDN] |= LDN_BUTTON;

            buttons[ANT] <<= 1;
            buttons[ANT] |= ANT_BUTTON;
            
            TIMER5_ON = 1; // restart timer
        }
    }
*/
#endif /* _BUTTONS_H_ */