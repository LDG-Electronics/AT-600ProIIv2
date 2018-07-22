#include "includes.h"

#include "peripherals/config.h"
#include "ui.h"
#include <xc.h>

// Disable "unused variable" warning
#pragma warning disable 1090

/* ************************************************************************** */

void main(void) {
    startup();

    ui_mainloop();
}