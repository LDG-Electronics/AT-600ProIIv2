#include "pins.h"
#include "peripherals/pic_header.h"

/* ************************************************************************** */
/*  //! READ THIS FIRST

    This file uses very gritty macros in a very un-C-like style.
    First, history and context:

    Directly manipulating registers everywhere is messy and error-prone, and
    should be avoided if possible. At the same time, some GPIO still requires
    high-speed/low-latency access. An Arduino style abstract interface would be
    prohibitively slow(a single Arduino digitalWrite() compiles to something
    like 55 lines of assembly!), so that's out. The first version of this API,
    such that it is, was just #defined pin aliases:

    (for the non-datasheet-readers, on PIC18s you're supposed to read from PORT
    registers and write to LATCH registers)

    Legacy pin alias usage:
    #define SOME_INPUT_PIN PORTBbits.RB5 // reads from pin 5 on port B
    #define SOME_OUTPUT_PIN LATAbits.LATA6 // writes to pin 6 or port A

    These are fine from the usage perspective, and they're literally exactly as
    fast as direct access, but they really fall down at the maintenance end.
    There's a lot of opportunity for typos.

    The next evolution of this pin alias style defines a pair of macros that
    automagically expand a compact pin definition into the correct direct
    register manipulations.

    New pin alias macros:
    #define PORT(port, pin) PORT##port##bits.R##port##pin
    #define LATCH(port, pin) LAT##port##bits.LAT##port##pin

    New pin alias usage:
    #define SOME_INPUT_PIN PORT(B, 5)
    #define SOME_OUTPUT_PIN LATCH(A, 6)

    This style is the same in external code, but now its WAY easier to maintain.
    You can immediately see which port and pin you're talking about, and from
    experience, this is MUCH easier to port from project to project.

    The legacy pin alias style also required manual management of the other pin
    control registers. The legacy definitions of SOME_INPUT_PIN and 
    SOME_OUTPUT_PIN required matching statements in pins_init() like this:

    set each pin to the correct input or output status
    TRISBbits.TRISB5 = 1; // SOME_DIGITAL_INPUT_PIN
    TRISCbits.TRISC1 = 1; // SOME_ANALOG_INPUT_PIN
    TRISAbits.TRISA6 = 0; // SOME_OUTPUT_PIN

    enable analog input, if we need it
    ANSELCbits.ANSELC5 = 1; // SOME_ANALOG_INPUT_PIN

    enable pullup, if we need it
    WPUBbits.WPUB5 = 1; // SOME_DIGITAL_INPUT_PIN

    This is, quite simply, a mess.

    ! Start here if you don't need the history lesson
    pins.c uses a dirty macro trick to streamline the control register setup.
    Here we undefine the existing PORT() and LATCH() macros, and redefine a new
    version that reuses the already defined port and pin values.

    Lets take the previous example:
    #define SOME_INPUT_PIN PORT(B, 5)
    #define SOME_OUTPUT_PIN LATCH(A, 6)

    In the native form, they expand like this:
    "var = SOME_INPUT_PIN;" -> "var = PORTBbits.RB5"
    "SOME_OUTPUT_PIN = var;" -> "LATAbits.LATA6 = var;"

    If we undefine PORT() and PIN() and redefine them, like this:
    #undef PORT()
    #define PORT(port, pin) TRIS##port##bits.TRIS##port##pin = 1
    #undef LATCH()
    #define LATCH(port, pin) TRIS##port##bits.TRIS##port##pin = 0

    Then we can use the aliases like this:
    SOME_INPUT_PIN;
    SOME_OUTPUT_PIN;

    Which then expand like this:
    "SOME_INPUT_PIN;" -> "TRISBbits.TRISB5 = 1;"
    "SOME_OUTPUT_PIN;" -> "TRISAbits.TRISA6 = 0;"

    I will be the first to admit that is this looks pretty rough, and maybe
    something like:

    SOME_INPUT_PIN = INPUT;
    SOME_OUTPUT_PIN = OUTPUT;

    would be more C-like... but the more idiomatic solution requires manually
    duplicating state that is already embedded in the pin alias. Pin aliases
    that use the PORT() macro are all inputs, and ones that use LATCH() are all
    outputs. The style I've chosen IS pretty gross, but it also REALLY reduces
    the maintenance overhead and opportunity for a typo to send you on an hour
    long debugging journey.
*/

void pins_init(void) {
// --------------------------------------------------
// TRISx - TriState Control registers
// 0 - output enabled (0 stands for _o_utput)
// 1 - output disabled (1 stands for _i_nput)

// pins that use PORT are inputs
#undef PORT()
#define PORT(port, pin) TRIS##port##bits.TRIS##port##pin = 1

// pins that use LATCH are outputs
#undef LATCH()
#define LATCH(port, pin) TRIS##port##bits.TRIS##port##pin = 0
// --------------------------------------------------

    // Front panel button pins
    POWER_BUTTON_PIN;
    CDN_BUTTON_PIN;
    LUP_BUTTON_PIN;
    CUP_BUTTON_PIN;
    FUNC_BUTTON_PIN;
    LDN_BUTTON_PIN;
    ANT_BUTTON_PIN;
    TUNE_BUTTON_PIN;

    // Bargraph bitbang SPI pins
    FP_CLOCK_PIN;
    FP_DATA_PIN;
    FP_STROBE_PIN;

    // Status LED pins
    POWER_LED_PIN;
    BYPASS_LED_PIN;
    ANT_LED_PIN;

    // RF Sensor pins
    FWD_PIN;
    REV_PIN;
#ifdef DEVELOPMENT
    FREQ_PIN;
#endif

    // Radio interface input pin
    RADIO_CMD_PIN;

    // Relay driver bitbang SPI pins
    RELAY_CLOCK_PIN;
    RELAY_DATA_PIN;
    RELAY_STROBE_PIN;

    // Meter port pins
    METER_TX_PIN;
    METER_RX_PIN;

    // Debug UART pins
    DEBUG_TX_PIN;
    DEBUG_RX_PIN;

// --------------------------------------------------
// ANSELx - Analog Select registers
// 0 - analog disabled
// 1 - analog enabled

// Only input pins(which use PORT) can possibly be analog inputs
#undef PORT()
#define PORT(port, pin) ANSEL##port##bits.ANSEL##port##pin = 1
// --------------------------------------------------

    // RF Sensor pins
    FWD_PIN;
    REV_PIN;

// --------------------------------------------------
// WPUx - weak pullup enable registers
// 0 - pullup disabled
// 1 - pullup enabled

// Only input pins(which use PORT) need pullups right now
#undef PORT()
#define PORT(port, pin) WPU##port##bits.WPU##port##pin = 1
// --------------------------------------------------

    // Front panel button pins
    POWER_BUTTON_PIN;
    CDN_BUTTON_PIN;
    LUP_BUTTON_PIN;
    CUP_BUTTON_PIN;
    FUNC_BUTTON_PIN;
    LDN_BUTTON_PIN;
    ANT_BUTTON_PIN;
    TUNE_BUTTON_PIN;
}

// make sure our redefinitions don't somehow leak out of this file
#undef PORT()
#undef LATCH()