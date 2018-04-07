#ifndef _INTERRUPT_H_
#define _INTERRUPT_H_

/* ************************************************************************** */

// 
extern void interrupt_init(void);

// Macros to control interrupt system
#define begin_critical_section() (INTCON0bits.GIE = 0)
#define end_critical_section() (INTCON0bits.GIE = 1)

#endif