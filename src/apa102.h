#ifndef _APA102_H_
#define _APA102_H_

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
    struct {
        uint8_t control;
        uint24_t color;
    };
    uint32_t all;
}led_struct_t;

#define NUMBER_OF_LEDS 64
#define LED_OVERHEAD_FRAMES 2
// #define LED_BUFFER_SIZE (NUMBER_OF_LEDS + LED_OVERHEAD_FRAMES)
#define LED_BUFFER_SIZE (NUMBER_OF_LEDS)

#define START_FRAME_INDEX 0
#define END_FRAME_INDEX (LED_BUFFER_SIZE - 1)

#define START_FRAME_CONTENTS 0
#define END_FRAME_CONTENTS -1

extern led_struct_t led_buffer[LED_BUFFER_SIZE];

/* ************************************************************************** */

extern void apa102_init(void);
extern void apa_update(void);

/* -------------------------------------------------------------------------- */

#define _RED 0x010000
#define _BLUE 0x000001
#define _GREEN 0x000100

#define _VIOLET 0x010001
#define _CYAN 0x010100
#define _YELLOW 0x000101

#define _WHITE 0x010101
#define _OFF 0x000000

extern void apa_set_LED(uint8_t led, uint24_t color);
extern void apa_fill(uint24_t color);

#endif