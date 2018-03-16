#ifndef _DELAY_H_
#define _DELAY_H_

/* ************************************************************************** */

extern void delay_init(void);

// Actual delay is roughly (n + 2) useconds
extern void delay_us(uint16_t useconds);

// Actual delay is roughly (10n + 5) useconds
extern void delay_10us(uint16_t useconds);

// Actual delay is roughly (100n + 5) useconds
extern void delay_100us(uint16_t useconds);

// Actual delay is roughly (1000n + 5) useconds
extern void delay_ms(uint16_t mseconds);

#endif /* _DELAY_H_ */
