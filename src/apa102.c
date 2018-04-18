#include "includes.h"
#include "apa102.h"
#include "spi.h"
#include "pps.h"

/* ************************************************************************** */

led_struct_t led_buffer[LED_BUFFER_SIZE];

/* ************************************************************************** */

void apa102_enable(void)
{
    CLC3CONbits.EN = 1; // turn it on
    CLC4CONbits.EN = 1; // turn it on
}

void apa102_disable(void)
{
    CLC3CONbits.EN = 0; // turn it on
    CLC4CONbits.EN = 0; // turn it on
}

void apa_clc_init(void)
{
    // CLC Setup
    CLC3GLS0 = 0b00000010;
    CLC3GLS1 = 0b00001000;
    CLC3GLS2 = 0b00100000;
    CLC3GLS3 = 0b10000000;

    CLC3SEL0 = 0b101100; // CLC1 data 0, input is SPI Clock
    CLC3SEL1 = 0b101100; // CLC1 data 1, 
    CLC3SEL2 = 0b101100; // CLC1 data 2, 
    CLC3SEL3 = 0b101100; // CLC1 data 3, 
    
    CLC3POL = 0b00000000; // don't invert anything
    
    CLC3CONbits.MODE = 0b010; // 4-input AND mode

    // CLC Setup
    CLC4GLS0 = 0b00000010;
    CLC4GLS1 = 0b00001000;
    CLC4GLS2 = 0b00100000;
    CLC4GLS3 = 0b10000000;

    CLC4SEL0 = 0b101011; // CLC1 data 0, input is SPI Data
    CLC4SEL1 = 0b101011; // CLC1 data 1, 
    CLC4SEL2 = 0b101011; // CLC1 data 2, 
    CLC4SEL3 = 0b101011; // CLC1 data 3, 
    
    CLC4POL = 0b00000000; // don't invert anything
    
    CLC4CONbits.MODE = 0b010; // 4-input AND mode
}

void apa102_init(void)
{
    /*  apa102.c is currently a test feature, piggybacking onto the AT-600ProII
        project. The SPI peripheral is already initialized somewhere else, so
        don't initialize it here.
    */
    // spi_init();
    /*  apa102.c hijacks the existing SPI driver in the AT-600ProII by adding a
        second reroute-via-CLC for the SPI Clock and SPI Data lines
    */
    apa_clc_init();

    // PPS setup
    RD1PPS = PPS_CLC3OUT; // SCK via CLC3OUT
    RD0PPS = PPS_CLC4OUT; // SDO via CLC4OUT

    // Fill in start frame
    // led_buffer[START_FRAME_INDEX].all = START_FRAME_CONTENTS;

    // initialize the LED array
    for(uint8_t i = 0; i < NUMBER_OF_LEDS; i++)
    {
        led_buffer[i].control = 0b11111111;
        led_buffer[i].color = _OFF;
    }

    // Fill in end frame
    // led_buffer[END_FRAME_INDEX].all = END_FRAME_CONTENTS;

    TRISDbits.TRISD0 = 0;
    TRISDbits.TRISD1 = 0;

    apa102_enable();
    apa_update();
}

void apa_update(void)
{
    uint32_t controlFrame = 0;
    uint32_t startFrame = START_FRAME_CONTENTS;
    uint32_t endFrame = END_FRAME_CONTENTS;

    // start frame - four bytes of 0s
    // spi_tx_string((unsigned char *)startFrame, 4);

    // led_buffer is technically an array of 4-byte long structs, but if we
    // cast it to uchar*, we can treat it like a normal string
    spi_tx_string((unsigned char *)led_buffer, (LED_BUFFER_SIZE * 4));

    // end frame - four bytes of s
    spi_tx_string((unsigned char *)END_FRAME_CONTENTS, 4);
}

/* -------------------------------------------------------------------------- */

void apa_set_LED(uint8_t led, uint24_t color)
{
    led = led % NUMBER_OF_LEDS;

    led_buffer[led].color = color;
}

void apa_fill(uint24_t color)
{
    uint8_t i = 0;

    while(i < NUMBER_OF_LEDS)
    {
        led_buffer[i++].color = color;
    }
}