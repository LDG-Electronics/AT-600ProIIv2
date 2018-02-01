#include "includes.h"

// Button timing constants, in 1ms increments.
#define BTN_PRESS_DEBOUNCE  10
#define BTN_PRESS_SHORT     400
#define BTN_PRESS_MEDIUM    2500
#define BTN_PRESS_LONG      10000

#define SLEEP_THRESH        2000

void main(void) {
    uint16_t buttonCount = 0;
    uint16_t sleepCounter = SLEEP_THRESH;

    uint8_t TuneIsHeld = 0;
    uint8_t FuncIsHeld = 0;
    uint8_t FuncHoldProcessed = 0;

    startup();

    print_format(BRIGHT, RED);
    print_str_ln("Hello!");


    LED_A = 0;
    LED_B = 0;
    LED_C = 0;
    LED_D = 0;

    TRISBbits.TRISB5 = 0;
    LATBbits.LATB5 = 1;    
    TRISBbits.TRISB4 = 0;
    LATBbits.LATB4 = 1;    

    print_str_ln("hello");


    while(1)
    {
        print_str_ln("beep");

        LATA3 = 1;
        delay_ms(500);
        LATA3 = 0;
        delay_ms(500);
    }


    while(1)
    {
        print_int(get_freq());
        print_ln();
    }

    while(1)
    {
        LED_D = 0;
        LED_A = 1;
        __delay_ms(500);

        LED_A = 0;
        LED_B = 1;
        __delay_ms(500);

        LED_B = 0;
        LED_C = 1;
        __delay_ms(500);

        LED_C = 0;
        LED_D = 1;
        __delay_ms(500);
    }


    while (1)
    {
        // Button processing        

        if (get_buttons()) {
            sleepCounter = SLEEP_THRESH;
            if (buttonCount < 0xffff) buttonCount++;

            if (buttonCount >= BTN_PRESS_DEBOUNCE)
            {
                if ((FuncIsHeld == 0) && 
                    (TuneIsHeld == 0))
                {
                    if (btn_is_down(TUNE)) TuneIsHeld = 1;
                    if (btn_is_down(FUNC)) FuncIsHeld = 1;
                }
                if (FuncIsHeld == 1) {
                    if (btn_is_down(CDN)) {
                        FuncHoldProcessed = 1;

                        BYPASS_LED = 0;
                    }
                    if (btn_is_down(LUP)) {
                        FuncHoldProcessed = 1;

                        // empty
                    }
                    if (btn_is_down(LDN)) {
                        FuncHoldProcessed = 1;

                        // empty
                    }
                    if (btn_is_down(CUP)) {
                        FuncHoldProcessed = 1;

                        // empty
                    }
                    if (btn_is_down(TUNE)) {
                        FuncHoldProcessed = 1;

                        // empty
                    }
                }
                if (btn_is_down(TUNE))
                {
                    if (buttonCount < BTN_PRESS_DEBOUNCE) {
                        // button was not held long enough, do nothing
                    } else if (buttonCount < BTN_PRESS_SHORT) {
                        led_off();
                    } else if (buttonCount < BTN_PRESS_MEDIUM) {
                        led_on();
                    } else if (buttonCount < BTN_PRESS_LONG) {
                        led_off();
                    } else if (buttonCount >= BTN_PRESS_LONG) {
                        // button was held for too long, time out
                    }
                }
                if ((FuncIsHeld == 0) && (TuneIsHeld == 0))
                {
                    if (btn_is_down(CUP)) {
                        //empty
                    } else if (btn_is_down(CDN)) {
                        //empty
                    } else if (btn_is_down(LUP)) {
                        //empty
                    } else if (btn_is_down(LDN)) {
                        //empty
                    }
                }
            }
        } else {
            if (buttonCount >= BTN_PRESS_DEBOUNCE)
            {
                if (TuneIsHeld == 1) {
                    TuneIsHeld = 0;
                    if (buttonCount < BTN_PRESS_DEBOUNCE) {
                        // button was not held long enough, do nothing
                    } else if (buttonCount < BTN_PRESS_SHORT) {
                        //empty
                    } else if (buttonCount < BTN_PRESS_MEDIUM) {
                        //empty
                    } else if (buttonCount < BTN_PRESS_LONG) {
                        //empty
                    } else if (buttonCount >= BTN_PRESS_LONG) {
                        // button was held for too long, do nothing
                    }
                }
                
                if ((FuncIsHeld == 1) &&
                    (FuncHoldProcessed == 0))
                {
                    FuncIsHeld = 0;
                }
                FuncHoldProcessed = 0;
            }
            
            BYPASS_LED = 1;
            ANT_LED = 1;

            buttonCount = 0;
            
            sleepCounter--;
            // if (sleepCounter == 0) shutdown();
        }
        __delay_ms(1);
    }
}

