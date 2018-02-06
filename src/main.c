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

    //print_format(BRIGHT, RED);
    //print_str_ln("Hello!");

    while(1)
    {
        BYPASS_LED = 1;
        ANT_LED = 1;
        delay_ms(500);
        BYPASS_LED = 0;
        ANT_LED = 0;
        delay_ms(500);
    }
}

