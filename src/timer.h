#ifndef _TIMER_H_
#define _TIMER_H_

/* ************************************************************************** */

// Timer0 is used in the stopwatch module
// Timer1 is unused
// Timer2 is used for the systick module
// Timer3 is used in the frequency counter
// Timer4 is unused
// Timer5 is unused
// Timer6 is used for the button debounce subsystem

/* ************************************************************************** */

#define TIMER0_IF PIR3bits.TMR0IF
#define TIMER1_IF PIR4bits.TMR1IF
#define TIMER2_IF PIR4bits.TMR2IF
#define TIMER3_IF PIR6bits.TMR3IF
#define TIMER4_IF PIR7bits.TMR4IF
#define TIMER5_IF PIR8bits.TMR5IF
#define TIMER6_IF PIR9bits.TMR6IF

#define TIMER0_ON T0CON0bits.EN
#define TIMER1_ON T1CONbits.ON
#define TIMER2_ON T2CONbits.ON
#define TIMER3_ON T3CONbits.ON
#define TIMER4_ON T4CONbits.ON
#define TIMER5_ON T5CONbits.ON
#define TIMER6_ON T6CONbits.ON

#define TIMER1_GATE_EN T1GCONbits.GE
#define TIMER3_GATE_EN T3GCONbits.GE
#define TIMER5_GATE_EN T5GCONbits.GE

/* ************************************************************************** */

// Setup and control function for hardware timers.
// Each timerX_clear() can be customized for desired timer duration.

// Any additional or specialized timer functions should follow the existing
// naming convention of: timerX_verb()

// Timer0 control functions
#define timer0_start() TIMER0_ON = 1
#define timer0_stop() TIMER0_ON = 0
#define timer0_IF_clear() TIMER0_IF = 0
#define timer0_IF_read() TIMER0_IF
#define timer0_clear() TMR0H = 0x00; TMR0L = 0x00;
#define timer0_read() ((TMR0H << 8) | TMR0L)

// Timer1 control functions
#define timer1_start() TIMER1_ON = 1
#define timer1_stop() TIMER1_ON = 0
#define timer1_gate_enable() TIMER1_GATE_EN = 1
#define timer1_gate_disable() TIMER1_GATE_EN = 0
#define timer1_IF_clear() TIMER1_IF = 0
#define timer1_IF_read() TIMER1_IF
#define timer1_clear() TMR1H = 0x00; TMR1L = 0x00;
#define timer1_read() ((TMR1H << 8) | TMR1L)

// Timer2 control functions
#define timer2_start() TIMER2_ON = 1
#define timer2_stop() TIMER2_ON = 0
#define timer2_IF_clear() TIMER2_IF = 0
#define timer2_IF_read() TIMER2_IF
#define timer2_clear() TMR2 = 0x00;
#define timer2_read() TMR2

// Timer3 control functions
#define timer3_start() TIMER3_ON = 1
#define timer3_stop() TIMER3_ON = 0
#define timer3_gate_enable() TIMER3_GATE_EN = 1
#define timer3_gate_disable() TIMER3_GATE_EN = 0
#define timer3_IF_clear() TIMER3_IF = 0
#define timer3_IF_read() TIMER3_IF
#define timer3_clear() TMR3H = 0x00; TMR3L = 0x00;
#define timer3_read() ((TMR3H << 8) | TMR3L)

// Timer4 control functions
#define timer4_start() TIMER4_ON = 1
#define timer4_stop() TIMER4_ON = 0
#define timer4_IF_clear() TIMER4_IF = 0
#define timer4_IF_read() TIMER4_IF
#define timer4_clear() TMR4 = 0x00;
#define timer4_read() TMR4

// Timer5 control functions
#define timer5_start() TIMER5_ON = 1
#define timer5_stop() TIMER5_ON = 0
#define timer5_gate_enable() TIMER5_GATE_EN = 1
#define timer5_gate_disable() TIMER5_GATE_EN = 0
#define timer5_IF_clear() TIMER5_IF = 0
#define timer5_IF_read() TIMER5_IF
#define timer5_clear() TMR5H = 0x00; TMR5L = 0x00;
#define timer5_read() ((TMR5H << 8) | TMR5L)

// Timer6 control functions
#define timer6_start() TIMER6_ON = 1
#define timer6_stop() TIMER6_ON = 0
#define timer6_IF_clear() TIMER6_IF = 0
#define timer6_IF_read() TIMER6_IF
#define timer6_clear() TMR6 = 0x00;
#define timer6_read() TMR6

#endif
