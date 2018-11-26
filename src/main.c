#include "hardware.h"
#include "peripherals/config.h"
#include "ui.h"

/* ************************************************************************** */

// Disable warning spam about reset vectors
#pragma warning disable 2020

/* ************************************************************************** */

void main(void) {
    startup();

    ui_mainloop();
}