#include "includes.h"
#include <xc.h>
#include "peripherals/config.h"

/* ************************************************************************** */

void main(void)
{
    startup();

    ui_mainloop();
}