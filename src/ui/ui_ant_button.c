#include "display.h"
#include "events.h"
#include "os/buttons.h"
#include "ui_idle_block.h"

/* ************************************************************************** */
/*  Handler for holding ANT button

*/
void ant_hold(void) {
    toggle_antenna();
    show_antenna();
    update_status_LEDs();

    while (btn_is_down(ANT)) {
        ui_idle_block();
    }
    blink_antenna();
}