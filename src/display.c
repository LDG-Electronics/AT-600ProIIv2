#include "includes.h"

/* ************************************************************************** */

const uint8_t swrThreshDisplay[4] = { 0x08,0x10,0x20,0x40 };

const uint16_t ledBarTable[9] = { 0x00,0x01,0x03,0x07,0x0f,0x1f,0x3f,0x7f,0xff, };
const uint16_t ledSingleTable[9] = { 0x00,0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80, };

// Shifts out a 16-bit data value to the TPIC_595 chip(s)
// Requires that STROBE_PIN, CLOCK_PIN, and DATA_PIN are defined in hardware.h
void delay_3_nops(void)
{
    #asm
        nop
        nop
        nop
    #endasm;
}

void shift_out_595_16bit(uint16_t d)
{
    uint8_t i;
    (FP_STROBE_PIN) = 1;
    (FP_CLOCK_PIN) = 0;
    for (i = 0; i < 16; i++)
    {
        if (d & (1 << (15 - i)))
        {
            (FP_DATA_PIN) = 1;
        }
        else
        {
            (FP_DATA_PIN) = 0;
        }
        delay_3_nops();
        (FP_CLOCK_PIN) = 1;
        delay_3_nops();
        (FP_CLOCK_PIN) = 0;
        delay_3_nops();
    }
    (FP_STROBE_PIN) = 0;
    delay_3_nops();
    (FP_STROBE_PIN) = 1;
    delay_3_nops();
}

/* ************************************************************************** */

void display_init(void)
{
    // Clear Front Panel bitbang SPI pins
    FP_CLOCK_PIN = 0;
    FP_DATA_PIN = 0;
    FP_STROBE_PIN = 0;

    show_startup();

    ANT_LED = 1;
    BYPASS_LED = 1;
}

/* -------------------------------------------------------------------------- */

void show_startup(void)
{
    shift_out_595_16bit(0x0101);
    __delay_ms(100);
    shift_out_595_16bit(0x0202);
    __delay_ms(100);
    shift_out_595_16bit(0x0404);
    __delay_ms(75);
    shift_out_595_16bit(0x0808);
    __delay_ms(75);
    shift_out_595_16bit(0x1010);
    __delay_ms(75);
    shift_out_595_16bit(0x2020);
    __delay_ms(50);
    shift_out_595_16bit(0x4040);
    __delay_ms(50);
    shift_out_595_16bit(0x8080);
    __delay_ms(50);
    shift_out_595_16bit(0x0000);
}
