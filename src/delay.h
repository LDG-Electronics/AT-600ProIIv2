#ifndef _DELAY_H_
#define _DELAY_H_

/* ************************************************************************** */

extern void delay_init(void);

// delay_us() uses timer0 to wait n microseconds, to an accuracy of +-2%
extern void delay_us(uint16_t useconds);

/*  delay_ms() uses the system tick to wait between n-1 and n milliseconds.
    This error is caused by the 1ms resolution of the systick. delay_ms() can
    start anywhere in the 'current' ms. Calling delay_ms() with arguments less
    than 10 can result in ~(1/(n+1))% error, as the jitter becomes a larger and
    larger fraction of the desired wait time.

    If a high-accuracy delay is required, please use delay_us() instead.
*/
extern void delay_ms(uint16_t mseconds);

#endif
