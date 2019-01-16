#ifndef _UI_IDLE_BLOCK_H_
#define _UI_IDLE_BLOCK_H_

#include <stdint.h>

/* ************************************************************************** */
/*  RFhistory is used to 'debounce' the presence or absence of RF, as if it were
    a button.

    These are macro versions of the 4 state-checking functions from the button
    debouncing subsystem.
*/
extern uint8_t RFhistory;

#define clear_RF_history() RFhistory = 0
#define RF_is_present() (RFhistory == 0b11111111)
#define RF_is_absent() (RFhistory == 0b00000000)

/* ************************************************************************** */

extern void disable_auto_tuning(void);
extern void enable_auto_tuning(void);

/* ************************************************************************** */

extern void ui_idle_block(void);

#endif
