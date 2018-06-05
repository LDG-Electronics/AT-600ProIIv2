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
#define SUBMENU_TIMEOUT 2200

void threshold_submenu(void)
{
    uint16_t modeCount = SUBMENU_TIMEOUT;
    
    blink_thresh(3);
    show_thresh();
    
    while (1)
    {
        if (btn_is_pressed(LDN))
        {
            modeCount = SUBMENU_TIMEOUT;
            SWR_threshold_increment();
            
            show_thresh();
            blink_thresh(2);
            show_thresh();
        }        
        
        if (btn_is_pressed(FUNC)) break;
        modeCount--;
        if (modeCount == 0) break;

        delay_ms(1);
        system_idle_block();
    }
}

void function_submenu(void)
{
    uint16_t modeCount = SUBMENU_TIMEOUT; 

    while(btn_is_down(FUNC)); // make sure FUNC is released before we continue

    // TODO: There's a bug here that causes the animation to always be
    //       interrupted. Not sure why.
    
    play_interruptable_animation(&arrow_up);

    while (1)
    {
        if (btn_is_pressed(CUP)) {
            toggle_peak();
            show_peak();
            return;
        }
        if (btn_is_pressed(LUP)) {
            toggle_scale();
            blink_scale(4);
            return;
        }
        if (btn_is_pressed(CDN)) {
            toggle_auto();
            blink_auto(4);
            return;
        }
        if (btn_is_pressed(LDN)) {
            threshold_submenu();
            blink_thresh(4);
            return;
        }
        if (btn_is_pressed(ANT)) { 
            toggle_hiloz();
            blink_HiLoZ(4);
            return;
        }
        if (btn_is_pressed(TUNE)) {
            manual_store();
            return;
        }

        if (btn_is_pressed(POWER)) break;
        if (btn_is_pressed(FUNC)) break; // Pressing FUNC again cancels and exits
        modeCount--;
        if (modeCount == 0) break;

        delay_ms(1);
        system_idle_block();
    }
    play_animation(&arrow_down);
}

void shutdown_submenu(void)
{
    uint16_t holdCount = 0;

    #if LOG_LEVEL_MENUS > LOG_SILENT
    println("shutting down");
    #endif

    // Turn off the whole front panel
    clear_status_LEDs();
    display_clear();

    while(btn_is_down(POWER));
    delay_ms(100);

    while(1)
    {
        if (btn_is_down(POWER)) holdCount++;
        if (holdCount == 1000) break;

        delay_ms(1);
        system_idle_block();
    }

    #if LOG_LEVEL_MENUS > LOG_SILENT
    println("Hello again!");
    #endif
    
    // Put the Status LEDs back how we found them
    update_status_LEDs();

    while(btn_is_down(POWER));
}

/* -------------------------------------------------------------------------- */

#define RELAY_INCREMENT_COUNT 4
#define RELAY_INCREMENT_FAST_DELAY 75
#define RELAY_INCREMENT_SLOW_DELAY 200

void cup_hold(void)
{
    uint16_t holdCount = 0;

    while(btn_is_down(CUP))
    {
        if (holdCount < UINT16_MAX) holdCount++;

        capacitor_increment();

        if(holdCount > RELAY_INCREMENT_COUNT) {
            delay_ms(RELAY_INCREMENT_FAST_DELAY);
        } else {
            delay_ms(RELAY_INCREMENT_SLOW_DELAY);
        }

        system_idle_block();
    }
    save_flags();
}

void cdn_hold(void)
{
    uint16_t holdCount = 0;

    while(btn_is_down(CDN))
    {
        if (holdCount < UINT16_MAX) holdCount++;

        capacitor_decrement();

        if(holdCount > RELAY_INCREMENT_COUNT) {
            delay_ms(RELAY_INCREMENT_FAST_DELAY);
        } else {
            delay_ms(RELAY_INCREMENT_SLOW_DELAY);
        }

        system_idle_block();
    }
    save_flags();
}

void lup_hold(void)
{
    uint16_t holdCount = 0;

    while(btn_is_down(LUP))
    {
        if (holdCount < UINT16_MAX) holdCount++;

        inductor_increment();

        if(holdCount > RELAY_INCREMENT_COUNT) {
            delay_ms(RELAY_INCREMENT_FAST_DELAY);
        } else {
            delay_ms(RELAY_INCREMENT_SLOW_DELAY);
        }

        system_idle_block();
    }
    save_flags();
}

void ldn_hold(void)
{
    uint16_t holdCount = 0;

    while(btn_is_down(LDN))
    {
        if (holdCount < UINT16_MAX) holdCount++;

        inductor_decrement();

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
    uint16_t holdCount = 0;

    while(btn_is_down(TUNE))
    {
        if (holdCount < UINT16_MAX) holdCount++;

        if (holdCount < BTN_PRESS_DEBOUNCE) {
            // button was not held long enough, do nothing
        } else if (holdCount < BTN_PRESS_SHORT) {
            display_single_frame(&center_crawl, 0);
        } else if (holdCount < BTN_PRESS_MEDIUM) {
            display_single_frame(&center_crawl, 2);
        } else if (holdCount < BTN_PRESS_LONG) {
            display_single_frame(&center_crawl, 3);
        } else if (holdCount >= BTN_PRESS_LONG) {
            display_clear();
        }

        delay_ms(1);
        system_idle_block();
    }

    display_clear();

    if (holdCount < BTN_PRESS_DEBOUNCE) {
        // button was not held long enough, do nothing
    } else if (holdCount < BTN_PRESS_SHORT) {
        toggle_bypass();
        blink_bypass();
    } else if (holdCount < BTN_PRESS_MEDIUM) {
        // request_memory_tune();
        request_full_tune();
    } else if (holdCount < BTN_PRESS_LONG) {
        request_full_tune();
    } else if (holdCount >= BTN_PRESS_LONG) {
        // button was held for too long, do nothing
    }
    save_flags();
}

void func_hold(void)
{
    uint8_t FuncHoldProcessed = 0;

    while(btn_is_down(FUNC))
    {
        if (btn_is_pressed(CUP)) {
            FuncHoldProcessed = 1;
            show_peak();
        }
        if (btn_is_pressed(CDN)) {
            FuncHoldProcessed = 1;
            blink_auto(3);
            show_auto();
        }
        if (btn_is_pressed(LDN)) {
            FuncHoldProcessed = 1;
            blink_thresh(3);
            show_thresh();
        }
        if (btn_is_pressed(LUP)) {
            FuncHoldProcessed = 1;
            blink_scale(3);
            show_scale();
        }
        if (btn_is_pressed(ANT)) {
            FuncHoldProcessed = 1;
            blink_HiLoZ(2);
            show_HiLoZ();
        }

        delay_ms(1);
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

    // This loop ensures that the antenna is only toggled once per button press
    while(btn_is_down(ANT))
    {
        delay_ms(1);
        system_idle_block();
    }
    save_flags();
}

void power_hold(void)
{
    uint16_t holdCount = 0;

    while(btn_is_down(POWER))
    {
        if (holdCount < UINT16_MAX) holdCount++;

        if (holdCount == 1500) {
            shutdown_submenu();
            break;
        }

        delay_ms(1);
        system_idle_block();
    }
    save_flags();
    delay_ms(25);
}