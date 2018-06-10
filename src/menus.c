#include "includes.h"

/* ************************************************************************** */

// Button timing constants, in 1ms increments.
#define BTN_PRESS_DEBOUNCE  20
#define BTN_PRESS_SHORT     350
#define BTN_PRESS_MEDIUM    2000
#define BTN_PRESS_LONG      10000

/* ************************************************************************** */
/*  Notes on the system idle block
    This function contains various 'background' activities that should be
    periodically serviced when the system isn't doing anything else important.
*/
void system_idle_block(void)
{
    shell_update();
    task_manager_update();
}

/* ************************************************************************** */
#define SUBMENU_DURATION 2200

void threshold_blink_and_hold(uint8_t blinks)
{
    blink_thresh(blinks); // blink for emphasis
    show_thresh(); // leave it on the screen
}

void threshold_submenu(void)
{
    uint24_t startTime = systick_read(); // stash the current time

    threshold_blink_and_hold(3);
    
    while(1)
    {
        if(btn_is_down(LDN)) {
            startTime = systick_read(); // reset the start time
            SWR_threshold_increment();
            
            threshold_blink_and_hold(2);
        }        
        
        // Pressing FUNC again cancels and exits
        if(btn_is_down(FUNC)) break;

        // Cancel and exit if it's been longer than SUBMENU_DURATION
        if(systick_elapsed_time(startTime) >= SUBMENU_DURATION) break;

        system_idle_block();
    }
}

void function_submenu(void)
{
    uint24_t startTime = systick_read(); // stash the current time

    while(btn_is_down(FUNC)); // make sure FUNC is released before we continue

    delay_ms(50);
    play_interruptable_animation(&arrow_up);

    while(1)
    {
        if(btn_is_down(CUP)) {
            toggle_peak();
            show_peak();
            return;
        }
        if(btn_is_down(LUP)) {
            toggle_scale();
            blink_scale(4);
            return;
        }
        if(btn_is_down(CDN)) {
            toggle_auto();
            blink_auto(4);
            return;
        }
        if(btn_is_down(LDN)) {
            threshold_submenu();
            blink_thresh(4);
            return;
        }
        if(btn_is_down(ANT)) { 
            toggle_hiloz();
            blink_HiLoZ(4);
            return;
        }
        if(btn_is_down(TUNE)) {
            manual_store();
            return;
        }

        // Pressing POWER or FUNC cancels and exits
        if(btn_is_down(POWER)) break;
        if(btn_is_down(FUNC)) break;
        
        // Cancel and exit if it's been longer than SUBMENU_DURATION
        if(systick_elapsed_time(startTime) >= SUBMENU_DURATION) break;

        system_idle_block();
    }
    play_animation(&arrow_down);
}

#define POWER_BUTTON_DURATION 1000

void shutdown_submenu(void)
{
    uint24_t startTime = systick_read(); // stash the current time

    #if LOG_LEVEL_MENUS > LOG_SILENT
    println("shutting down");
    #endif

    // Turn off the whole front panel
    clear_status_LEDs();
    display_clear();

    while(btn_is_down(POWER)); // wait until power button is released
    delay_ms(100);

    while(1)
    {
        if(systick_elapsed_time(startTime) >= POWER_BUTTON_DURATION) break;

        delay_ms(1);
        system_idle_block();
    }

    #if LOG_LEVEL_MENUS > LOG_SILENT
    println("Hello again!");
    #endif
    
    // Put the Status LEDs back how we found them
    update_status_LEDs();

    while(btn_is_down(POWER)); // wait until power button is released
}

/* -------------------------------------------------------------------------- */

#define RELAY_INCREMENT_COUNT 4
#define RELAY_INCREMENT_FAST_DELAY 75
#define RELAY_INCREMENT_SLOW_DELAY 200

void relay_button_hold(void)
{
    uint16_t holdCount = 0;

    // stay in loop while any relay button is held
    while(check_multiple_buttons(&btn_is_down, 4, CUP, CDN, LUP, LDN))
    {
        if(holdCount < UINT16_MAX) holdCount++;
        
        if(btn_is_down(CUP)) capacitor_increment();
        if(btn_is_down(CDN)) capacitor_decrement();
        if(btn_is_down(LUP)) inductor_increment();
        if(btn_is_down(LDN)) inductor_decrement();

        if(holdCount > RELAY_INCREMENT_COUNT) {
            delay_ms(RELAY_INCREMENT_FAST_DELAY);
        } else {
            delay_ms(RELAY_INCREMENT_SLOW_DELAY);
        }

        system_idle_block();
    }
    save_flags();
}

void tune_hold(void)
{
    uint24_t startTime = systick_read();
    uint24_t elapsedTime;

    while(btn_is_down(TUNE)) // stay in loop while TUNE is held
    {
        elapsedTime = systick_elapsed_time(startTime);

        if (elapsedTime < BTN_PRESS_DEBOUNCE) {
            // button was not held long enough, do nothing
        } else if (elapsedTime < BTN_PRESS_SHORT) {
            display_single_frame(&center_crawl, 0);
        } else if (elapsedTime < BTN_PRESS_MEDIUM) {
            display_single_frame(&center_crawl, 2);
        } else if (elapsedTime < BTN_PRESS_LONG) {
            display_single_frame(&center_crawl, 3);
        } else if (elapsedTime >= BTN_PRESS_LONG) {
            display_clear();
        }

        system_idle_block();
    }

    display_clear();

    if (elapsedTime < BTN_PRESS_DEBOUNCE) {
        // button was not held long enough, do nothing
    } else if (elapsedTime < BTN_PRESS_SHORT) {
        toggle_bypass();
        blink_bypass();
    } else if (elapsedTime < BTN_PRESS_MEDIUM) {
        // request_memory_tune();
        request_full_tune();
    } else if (elapsedTime < BTN_PRESS_LONG) {
        request_full_tune();
    } else if (elapsedTime >= BTN_PRESS_LONG) {
        // button was held for too long, do nothing
    }
    save_flags();
}

void func_hold(void)
{
    uint8_t FuncHoldProcessed = 0;

    while(btn_is_down(FUNC)) // stay in loop while FUNC is held
    {
        if(btn_is_down(CUP)) {
            FuncHoldProcessed = 1;
            show_peak();
        }
        if(btn_is_down(CDN)) {
            FuncHoldProcessed = 1;
            blink_auto(3);
            show_auto();
        }
        if(btn_is_down(LDN)) {
            FuncHoldProcessed = 1;
            blink_thresh(3);
            show_thresh();
        }
        if(btn_is_down(LUP)) {
            FuncHoldProcessed = 1;
            blink_scale(3);
            show_scale();
        }
        if(btn_is_down(ANT)) {
            FuncHoldProcessed = 1;
            blink_HiLoZ(2);
            show_HiLoZ();
        }

        system_idle_block();
    }
    if (FuncHoldProcessed == 0) function_submenu();
    save_flags();
}

void ant_hold(void)
{
    toggle_antenna();
    blink_antenna();
    update_antenna_LED();

    while(btn_is_down(ANT)) // stay in loop while ANT is held
    {
        system_idle_block();
    }
    save_flags();
}

#define POWER_HOLD_DURATION 1500

void power_hold(void)
{
    uint24_t startTime = systick_read();

    while(btn_is_down(POWER)) // stay in loop while POWER is held
    {
        if(systick_elapsed_time(startTime) >= POWER_HOLD_DURATION) break;

        system_idle_block();
    }
    save_flags();
    delay_ms(25);
}