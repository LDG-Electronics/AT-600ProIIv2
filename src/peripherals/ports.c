#include "ports.h"
#include "device_header.h"

/* ************************************************************************** */

void port_init(void) {
    // Tri-state control; 0 = output enabled, 1 = output disabled
    // Explicitly disable all outputs
    TRISA = 0xFF;
    TRISB = 0xFF;
    TRISC = 0xFF;
    TRISD = 0xFF;
    TRISE = 0xFF;
#ifdef TRISF
    TRISF = 0xFF;
#endif

    // Output latch - explicitly drive all outputs low
    LATA = 0;
    LATB = 0;
    LATC = 0;
    LATD = 0;
    LATE = 0;
#ifdef LATF
    LATF = 0;
#endif

    // Analog Select; 0 = analog mode is disabled, 1 = analog mode is enabled
    ANSELA = 0;
    ANSELB = 0;
    ANSELC = 0;
    ANSELD = 0;
    ANSELE = 0;
#ifdef ANSELF
    ANSELF = 0;
#endif

    // Weak Pull-up; 0 = pull-up disabled, 1 = pull-up enabled
    WPUA = 0;
    WPUB = 0;
    WPUC = 0;
    WPUD = 0;
    WPUE = 0;
#ifdef WPUF
    WPUF = 0;
#endif

    // Open-Drain Control; 0 = Output drives both high and low, 1 =
    ODCONA = 0;
    ODCONB = 0;
    ODCONC = 0;
    ODCOND = 0;
    ODCONE = 0;
#ifdef ODCONF
    ODCONF = 0;
#endif

    // Slew Rate Control; 0 = maximum slew rate, 1 = limited slew rate
    SLRCONA = 0;
    SLRCONB = 0;
    SLRCONC = 0;
    SLRCOND = 0;
    SLRCONE = 0;
#ifdef SLRCONF
    SLRCONF = 0;
#endif

    // Input (Logic) Level Control; 0 = TTL, 1 = Schmitt Trigger
    INLVLA = 0xff;
    INLVLB = 0xff;
    INLVLC = 0xff;
    INLVLD = 0xff;
    INLVLE = 0xff;
#ifdef INLVLF
    INLVLF = 0xff;
#endif
}