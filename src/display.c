#include "includes.h"

/* ************************************************************************** */
/*  APA-102C data structure

    Each led in a chain can be represented by one of these structs.

    The 'global' entry represents a global brightness setting.
    The three color entries represent the pwm duty cycle of that color.

    Example LED assignment block:

    led_buffer[1].brightness = 0;
    led_buffer[1].blue = 0;
    led_buffer[1].green = 0;
    led_buffer[1].red = 0;
*/
typedef union {
    struct {
        uint8_t brightness;
        uint8_t blue;
        uint8_t green;
        uint8_t red;
    };
    uint32_t all;
}led_struct_t;

#define NUMBER_OF_LEDS 1
#define LED_OVERHEAD_FRAMES 2
#define LED_BUFFER_SIZE (NUMBER_OF_LEDS + LED_OVERHEAD_FRAMES)

#define START_FRAME_INDEX 0
#define END_FRAME_INDEX (LED_BUFFER_SIZE - 1)

#define START_FRAME_CONTENTS 0
#define END_FRAME_CONTENTS -1

led_struct_t led_buffer[LED_BUFFER_SIZE];

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

    // Clear LED SPI pins
    CLOCK_PIN = 0;
    DATA_PIN = 0;

    // Fill in start frame
    led_buffer[START_FRAME_INDEX].all = START_FRAME_CONTENTS;

    // Fill in end frame
    led_buffer[END_FRAME_INDEX].all = END_FRAME_CONTENTS;

    show_startup();

    ANT_LED = 1;
    BYPASS_LED = 1;
}

void spi_bb_tx(uint8_t data)
{
    uint8_t i = 0;

    // sync up with clock
    while (TIMER6_IF == 0);
    TIMER6_IF = 0;            // clear bit

    for (i = 0; i < 8; i++)
    {
        if (data & (1 << (8 - i))) {
            DATA_PIN = 1;
        } else {
            DATA_PIN = 0;
        }
        CLOCK_PIN = 1;
        while (TIMER6_IF == 0);
        TIMER6_IF = 0;            // clear bit

        CLOCK_PIN = 0;
        while (TIMER6_IF == 0);
        TIMER6_IF = 0;            // clear bit
    }
}

void spi_bb_update(void)
{
    uint8_t i = 0;

    for (i = 0; i < LED_BUFFER_SIZE; i++)
    {
        spi_bb_tx(led_buffer[i].brightness);
        spi_bb_tx(led_buffer[i].blue);
        spi_bb_tx(led_buffer[i].green);
        spi_bb_tx(led_buffer[i].red);
    }
    DATA_PIN = 0;
    CLOCK_PIN = 0;
}

void bargraph_update(void)
{
    SPI1_Exchange8bitBuffer((unsigned char *)led_buffer, (LED_BUFFER_SIZE * 4), NULL);
}

/* -------------------------------------------------------------------------- */

void bargraph_off(void)
{
    led_buffer[1].brightness = 0b11111111;
    led_buffer[1].blue = 0;
    led_buffer[1].green = 0;
    led_buffer[1].red = 0;

    spi_bb_update();
}
void bargraph_red(void)
{
    led_buffer[1].brightness = 0b11111111;
    led_buffer[1].blue = 0;
    led_buffer[1].green = 0;
    led_buffer[1].red = 0xff;

    spi_bb_update();
}
void bargraph_blue(void)
{
    led_buffer[1].brightness = 0b11111111;
    led_buffer[1].blue = 0xff;
    led_buffer[1].green = 0;
    led_buffer[1].red = 0;

    spi_bb_update();
}
void bargraph_green(void)
{
    led_buffer[1].brightness = 0b11111111;
    led_buffer[1].blue = 0;
    led_buffer[1].green = 0xff;
    led_buffer[1].red = 0;

    spi_bb_update();
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
