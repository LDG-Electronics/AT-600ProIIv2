#ifndef _FIXED_VOLTAGE_REFERENCE_H_
#define _FIXED_VOLTAGE_REFERENCE_H_

#include "pic18f47k42.h"

/* ************************************************************************** */
/*  Temperature Indicator Module

    TODO - fill in this description
*/

#define temp_indicator_enable() FVRCONbits.TSEN = 1
#define temp_indicator_disable() FVRCONbits.TSEN = 0

#define temp_indicator_high_range() FVRCONbits.TSRNG = 1
#define temp_indicator_low_range() FVRCONbits.TSRNG = 0

/* -------------------------------------------------------------------------- */
/*  Fixed Voltage Reference

    The PIC18FXXK42 family has a Fixed Voltage Reference peripheral than can be
    used as a reference voltage in several of its analog peripherals.

    FVR outputs can be used by the following peripherals:
    ADC input Channel
    ADC positive reference
    Comparator input
    Digital-to-Analog Converter

    !IMPORTANT!
    The FVR needs time to stabilize whenever its settings are modified. Every
    call to fvr_set_xxx_buffer_gain() MUST be followed by a call to
    fvr_wait_until_ready().
*/

/* -------------------------------------------------------------------------- */

#define fvr_enable() FVRCONbits.EN = 1
#define fvr_disable() FVRCONbits.EN = 0

#define fvr_wait_until_ready() while (!FVRCONbits.RDY)

/* -------------------------------------------------------------------------- */
// FVR Gain controls
#define fvr_set_comparator_buffer_gain(value) FVRCONbits.CDAFVR = value
#define fvr_set_adc_buffer_gain(value) FVRCONbits.ADFVR = value

// FVR Gain values, use as arguments in fvr_set_xxx_buffer_gain()
#define FVR_GAIN_OFF 0b00 // Buffer is off
#define FVR_GAIN_1X 0b01  // Buffer set to 1.024V
#define FVR_GAIN_2X 0b10  // Buffer set to 2.048V
#define FVR_GAIN_4X 0b11  // Buffer set to 4.096V

#endif