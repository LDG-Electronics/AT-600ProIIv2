#include "oscillator.h"
#include "device_header.h"

/* ************************************************************************** */

void external_oscillator_init(void) {
    OSCCON1bits.NOSC = 0b111; // NOSC EXTOSC
    OSCFRQ = 0x08;            // HFFRQ 64_MHz;
}

void internal_oscillator_init(void) {
    OSCCON1 = 0x60; // NOSC HFINTOSC; NDIV 1;
    OSCCON3 = 0x00; // CSWHOLD may proceed; SOSCPWR Low power;
    OSCEN = 0x00;   // MFOEN disabled; LFOEN disabled; ADOEN disabled;
                    // SOSCEN disabled; EXTOEN disabled; HFOEN disabled;
    OSCFRQ = 0x08;  // HFFRQ 64_MHz;
    OSCTUNE = 0x00; // TUN 0;
}