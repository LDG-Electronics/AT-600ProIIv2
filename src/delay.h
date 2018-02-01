#ifndef _DELAY_H_
#define _DELAY_H_

/* ************************************************************************** */


/****************************************************************/
/* Built-in delay routine					*/
/****************************************************************/

/*
#pragma intrinsic(_delay)
extern __nonreentrant void _delay(unsigned long);
#pragma intrinsic(_delaywdt)
extern __nonreentrant void _delaywdt(unsigned long);
#pragma intrinsic(_delay3)
extern __nonreentrant void _delay3(unsigned char);
// NOTE: To use the macros below, YOU must have previously defined _XTAL_FREQ
#define __delay_us(x) _delay((unsigned long)((x)*(_XTAL_FREQ/4000000.0)))
#define __delay_ms(x) _delay((unsigned long)((x)*(_XTAL_FREQ/4000.0)))
#define __delaywdt_us(x) _delaywdt((unsigned long)((x)*(_XTAL_FREQ/4000000.0)))
#define __delaywdt_ms(x) _delaywdt((unsigned long)((x)*(_XTAL_FREQ/4000.0)))
*/


extern void delay_init(void);

// Actual delay is roughly (n + 2) useconds
// extern void delay_us(uint16_t useconds);

// Actual delay is roughly (10n + 5) useconds
extern void delay_10us(uint16_t useconds);

// Actual delay is roughly (100n + 5) useconds
extern void delay_100us(uint16_t useconds);

// Actual delay is roughly (1000n + 5) useconds
extern void delay_ms(uint16_t mseconds);

#endif
