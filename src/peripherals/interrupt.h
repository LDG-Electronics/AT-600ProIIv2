#ifndef _INTERRUPT_H_
#define _INTERRUPT_H_

#include "pic18f47k42.h"

/* ************************************************************************** */

// Macros to control interrupt system
#define begin_critical_section() (INTCON0bits.GIE = 0)
#define end_critical_section() (INTCON0bits.GIE = 1)

/* -------------------------------------------------------------------------- */

extern void interrupt_init(void);

#endif