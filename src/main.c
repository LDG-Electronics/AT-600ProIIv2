#include "config.h"
#include "hardware.h"
#include "ui/ui.h"

/* ************************************************************************** */

// Disable "unused variable" warning
#pragma warning disable 1090

// Disable warning spam about reset vectors
#pragma warning disable 2020

// Disable warning spam about unused functions
#pragma warning disable 520

/* ************************************************************************** */

void main(void) {
    startup();

    ui_mainloop();
}