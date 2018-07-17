#include "includes.h"
static uint8_t LOG_LEVEL = L_SILENT;

/* ************************************************************************** */
/*  Notes on the system idle block
    This function contains various 'background' activities that should be
    periodically serviced when the system isn't doing anything else important.
*/
void ui_idle_block(void) {
    shell_update();
    event_scheduler_update();
}

/* ************************************************************************** */

// Button timing constants, in 1ms increments.
#define BTN_PRESS_DEBOUNCE 20
#define BTN_PRESS_SHORT 350
#define BTN_PRESS_MEDIUM 2000
#define BTN_PRESS_LONG 10000

/* ************************************************************************** */
#define SUBMENU_DURATION 2200

void threshold_blink_and_hold(uint8_t blinks) {
    blink_thresh(blinks); // blink for emphasis
    show_thresh();        // leave it on the screen
}

void threshold_submenu(void) {
    LOG_TRACE(println("decode_escape_sequence"););
    threshold_blink_and_hold(3);

    system_time_t startTime = systick_read(); // stash the current time

    while (1) {
        if (btn_is_down(LDN)) {
            startTime = systick_read(); // reset the start time
            SWR_threshold_increment();

            threshold_blink_and_hold(2);
        }

        // Pressing FUNC again cancels and exits
        if (btn_is_down(FUNC))
            break;

        // Cancel and exit if it's been longer than SUBMENU_DURATION
        if (systick_elapsed_time(startTime) >= SUBMENU_DURATION)
            break;

        ui_idle_block();
    }
}

void function_submenu(void) {
    LOG_TRACE(println("function_submenu"););
    while (btn_is_down(FUNC)){}
        ; // make sure FUNC is released before we continue

    delay_ms(50);
    play_interruptable_animation(&arrow_up);

    system_time_t startTime = systick_read(); // stash the current time

    while (1) {
        if (btn_is_down(CUP)) {
            toggle_peak();
            show_peak();
            return;
        }
        if (btn_is_down(LUP)) {
            toggle_scale();
            blink_scale(4);
            return;
        }
        if (btn_is_down(CDN)) {
            toggle_auto();
            blink_auto(4);
            return;
        }
        if (btn_is_down(LDN)) {
            threshold_submenu();
            blink_thresh(4);
            return;
        }
        if (btn_is_down(ANT)) {
            toggle_hiloz();
            blink_HiLoZ(4);
            return;
        }
        if (btn_is_down(TUNE)) {
            manual_store();
            return;
        }

        // Pressing POWER or FUNC cancels and exits
        if (btn_is_down(POWER))
            break;
        if (btn_is_down(FUNC))
            break;

        // Cancel and exit if it's been longer than SUBMENU_DURATION
        if (systick_elapsed_time(startTime) >= SUBMENU_DURATION)
            break;

        ui_idle_block();
    }
    play_animation(&arrow_down);
}

#define POWER_BUTTON_DURATION 1000

void shutdown_submenu(void) {
    LOG_TRACE(println("shutdown_submenu"););
    LOG_INFO(println("shutting down"););

    // Turn off the whole front panel
    clear_status_LEDs();
    display_clear();

    while (btn_is_down(POWER))
        ; // wait until power button is released
    delay_ms(100);

    system_time_t startTime = systick_read(); // stash the current time

    while (1) {
        if (systick_elapsed_time(startTime) >= POWER_BUTTON_DURATION)
            break;

        delay_ms(1);
        ui_idle_block();
    }

    LOG_INFO(println("shutting down"););

    // Put the Status LEDs back how we found them
    update_status_LEDs();

    // wait until power button is released
    while (btn_is_down(POWER))
        ;
}

/* -------------------------------------------------------------------------- */

void relay_button_hold(void) {
    LOG_TRACE(println("relay_button_hold"););
    system_time_t currentTime = systick_read();
    uint8_t incrementCount = 0;
    uint8_t incrementDelay = 200;

    // stay in loop while any relay button is held
    while (check_multiple_buttons(&btn_is_down, 4, CUP, CDN, LUP, LDN)) {
        if (systick_elapsed_time(currentTime) >= incrementDelay) {
            currentTime = systick_read();

            // can't go up and down at the same time
            if (btn_is_down(CUP) && btn_is_down(CDN)) {
                // do nothing
            } else if (btn_is_down(CUP)) {
                capacitor_increment();
            } else if (btn_is_down(CDN)) {
                capacitor_decrement();
            }
            if (btn_is_down(LUP)) {
                inductor_increment();
            } else if (btn_is_down(LDN)) {
                inductor_decrement();
            } else if (btn_is_down(LUP) && btn_is_down(LDN)) {
                // do nothing
            }

            if (incrementCount < UINT8_MAX)
                incrementCount++;
            if (incrementCount == 4)
                incrementDelay = 75;
            if (incrementCount == 32)
                incrementDelay = 50;
        }
        ui_idle_block();
    }
    // lock_display();
    event_register("display_release", display_release, 1000);
    save_flags();
}

void tune_hold(void) {
    LOG_TRACE(println("tune_hold"););
    system_time_t elapsedTime;
    system_time_t startTime = systick_read();

    while (btn_is_down(TUNE)) // stay in loop while TUNE is held
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

        ui_idle_block();
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

void func_hold(void) {
    LOG_TRACE(println("func_hold"););
    uint8_t FuncHoldProcessed = 0;

    while (btn_is_down(FUNC)) // stay in loop while FUNC is held
    {
        if (btn_is_down(CUP)) {
            FuncHoldProcessed = 1;
            show_peak();
        }
        if (btn_is_down(CDN)) {
            FuncHoldProcessed = 1;
            blink_auto(3);
            show_auto();
        }
        if (btn_is_down(LDN)) {
            FuncHoldProcessed = 1;
            blink_thresh(3);
            show_thresh();
        }
        if (btn_is_down(LUP)) {
            FuncHoldProcessed = 1;
            blink_scale(3);
            show_scale();
        }
        if (btn_is_down(ANT)) {
            FuncHoldProcessed = 1;
            blink_HiLoZ(2);
            show_HiLoZ();
        }

        ui_idle_block();
    }
    if (FuncHoldProcessed == 0)
        function_submenu();
    save_flags();
}

void ant_hold(void) {
    LOG_TRACE(println("ant_hold"););
    toggle_antenna();
    blink_antenna();
    update_antenna_LED();

    while (btn_is_down(ANT)) // stay in loop while ANT is held
    {
        ui_idle_block();
    }
    save_flags();
}

#define POWER_HOLD_DURATION 1500

void power_hold(void) {
    LOG_TRACE(println("power_hold"););
    system_time_t startTime = systick_read();

    while (btn_is_down(POWER)) // stay in loop while POWER is held
    {
        if (systick_elapsed_time(startTime) >= POWER_HOLD_DURATION)
            break;

        ui_idle_block();
    }
    save_flags();
    delay_ms(25);
}

/* ************************************************************************** */

void ui_mainloop(void) {
    log_register();

    while (1) {
        // Relay buttons
        if (check_multiple_buttons(&btn_is_down, 4, CUP, CDN, LUP, LDN)) {
            relay_button_hold();
        }

        // Other buttons
        if (btn_is_down(FUNC))
            func_hold();
        if (btn_is_down(TUNE))
            tune_hold();
        if (btn_is_down(ANT))
            ant_hold();
        if (btn_is_down(POWER))
            power_hold();

        ui_idle_block();
    }
}