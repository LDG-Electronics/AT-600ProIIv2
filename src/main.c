#include "includes.h"

#include "peripherals/config.h"
#include <xc.h>

/* ************************************************************************** */

void main(void) {
    startup();

    ui_mainloop();
}