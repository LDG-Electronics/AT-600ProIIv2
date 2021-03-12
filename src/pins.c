#include "pins.h"
#include "peripherals/pic_header.h"

/* ************************************************************************** */
/* [[[cog
    from codegen import fmt; import pins
    cog.outl(fmt(pins.pin_definitions()))
]]] */

// GPIO read functions
bool read_POWER_BUTTON_PIN(void) { return PORTAbits.RA3; }
bool read_CDN_BUTTON_PIN(void) { return PORTAbits.RA4; }
bool read_LUP_BUTTON_PIN(void) { return PORTAbits.RA5; }
bool read_CUP_BUTTON_PIN(void) { return PORTBbits.RB2; }
bool read_FUNC_BUTTON_PIN(void) { return PORTBbits.RB4; }
bool read_LDN_BUTTON_PIN(void) { return PORTBbits.RB5; }
bool read_FREQ_PIN(void) { return PORTEbits.RE0; }
bool read_TUNE_BUTTON_PIN(void) {
#ifdef DEVELOPMENT
    return PORTEbits.RE1;
#else
    return PORTBbits.RB6;
#endif
}
bool read_ANT_BUTTON_PIN(void) {
#ifdef DEVELOPMENT
    return PORTEbits.RE2;
#else
    return PORTBbits.RB7;
#endif
}

// Button stuff
// array of pointers to button reading functions
button_function_t buttonFunctions[NUMBER_OF_BUTTONS] = {
    read_POWER_BUTTON_PIN, //
    read_CDN_BUTTON_PIN,   //
    read_LUP_BUTTON_PIN,   //
    read_CUP_BUTTON_PIN,   //
    read_FUNC_BUTTON_PIN,  //
    read_LDN_BUTTON_PIN,   //
    read_TUNE_BUTTON_PIN,  //
    read_ANT_BUTTON_PIN,   //
};

// GPIO write functions
void set_POWER_LED_PIN(bool value) { LATAbits.LATA2 = value; }
void set_FP_CLOCK_PIN(bool value) { LATAbits.LATA6 = value; }
void set_RADIO_CMD_PIN(bool value) { LATAbits.LATA7 = value; }
void set_ANT_LED_PIN(bool value) { LATBbits.LATB1 = value; }
void set_RELAY_CLOCK_PIN(bool value) { LATCbits.LATC0 = value; }
void set_RELAY_DATA_PIN(bool value) { LATCbits.LATC1 = value; }
void set_RELAY_STROBE_PIN(bool value) { LATCbits.LATC2 = value; }
void set_BYPASS_LED_PIN(bool value) { LATCbits.LATC3 = value; }
void set_FP_STROBE_PIN(bool value) { LATCbits.LATC4 = value; }
void set_FP_DATA_PIN(bool value) { LATCbits.LATC5 = value; }

// GPIO direction functions
void set_tris_BYPASS_LED_PIN(bool value) { TRISCbits.TRISC3 = value; }
void set_tris_FP_STROBE_PIN(bool value) { TRISCbits.TRISC4 = value; }
void set_tris_FP_DATA_PIN(bool value) { TRISCbits.TRISC5 = value; }

// [[[end]]]

/* ************************************************************************** */
/* [[[cog
    from codegen import fmt; import pins
    cog.outl(fmt(pins.pins_init()))
]]] */

void pins_init(void) {
    // FWD_PIN
    TRISAbits.TRISA0 = 1;
    ANSELAbits.ANSELA0 = 1;

    // REV_PIN
    TRISAbits.TRISA1 = 1;
    ANSELAbits.ANSELA1 = 1;

    // POWER_LED_PIN
    TRISAbits.TRISA2 = 0;

    // POWER_BUTTON_PIN
    TRISAbits.TRISA3 = 1;
    WPUAbits.WPUA3 = 1;

    // CDN_BUTTON_PIN
    TRISAbits.TRISA4 = 1;
    WPUAbits.WPUA4 = 1;

    // LUP_BUTTON_PIN
    TRISAbits.TRISA5 = 1;
    WPUAbits.WPUA5 = 1;

    // FP_CLOCK_PIN
    TRISAbits.TRISA6 = 0;

    // RADIO_CMD_PIN
    TRISAbits.TRISA7 = 0;

    // ANT_LED_PIN
    TRISBbits.TRISB1 = 0;

    // CUP_BUTTON_PIN
    TRISBbits.TRISB2 = 1;
    WPUBbits.WPUB2 = 1;

    // FUNC_BUTTON_PIN
    TRISBbits.TRISB4 = 1;
    WPUBbits.WPUB4 = 1;

    // LDN_BUTTON_PIN
    TRISBbits.TRISB5 = 1;
    WPUBbits.WPUB5 = 1;

    // RELAY_CLOCK_PIN
    TRISCbits.TRISC0 = 0;

    // RELAY_DATA_PIN
    TRISCbits.TRISC1 = 0;

    // RELAY_STROBE_PIN
    TRISCbits.TRISC2 = 0;

    // BYPASS_LED_PIN
    TRISCbits.TRISC3 = 0;

    // FP_STROBE_PIN
    TRISCbits.TRISC4 = 0;

    // FP_DATA_PIN
    TRISCbits.TRISC5 = 0;

    // USB_TX_PIN
    TRISCbits.TRISC6 = 0;

    // USB_RX_PIN
    TRISCbits.TRISC7 = 1;

    // FREQ_PIN
    TRISEbits.TRISE0 = 1;

// DEBUG_TX_PIN
#ifdef DEVELOPMENT
    TRISDbits.TRISD2 = 0;
#endif

// DEBUG_RX_PIN
#ifdef DEVELOPMENT
    TRISDbits.TRISD3 = 1;
#endif

// TUNE_BUTTON_PIN
#ifdef DEVELOPMENT
    TRISEbits.TRISE1 = 1;
#else
    TRISBbits.TRISB6 = 1;
#endif
#ifdef DEVELOPMENT
    WPUEbits.WPUE1 = 1;
#else
    WPUBbits.WPUB6 = 1;
#endif

// ANT_BUTTON_PIN
#ifdef DEVELOPMENT
    TRISEbits.TRISE2 = 1;
#else
    TRISBbits.TRISB7 = 1;
#endif
#ifdef DEVELOPMENT
    WPUEbits.WPUE2 = 1;
#else
    WPUBbits.WPUB7 = 1;
#endif
}
// [[[end]]]