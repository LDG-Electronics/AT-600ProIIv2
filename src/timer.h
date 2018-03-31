#ifndef _TIMER_H_
#define _TIMER_H_

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

#define TIMER0_H_REG TMR0H
#define TIMER0_L_REG TMR0L

/* ************************************************************************** */

// Timer0 is used in the stopwatch module
// Timer1 is unused
// Timer2 is used for the systick module
// Timer3 is used in the frequency counter
// Timer4 is unused
// Timer5 is used for the button debounce subsystem
// Timer6 is unused

/* ************************************************************************** */

// Setup and control function for hardware timers.
// Each timerX_init() and timerX_clear() can be customized for desired timer duration.

// Any additional or specialized timer functions should follow the existing
// naming convention of:  timerX_verb()

// Timer0 control functions
#define timer0_start() TIMER0_ON = 1
#define timer0_stop() TIMER0_ON = 0
#define timer0_IF_clear() TIMER0_IF = 0
#define timer0_IF_read() TIMER0_IF
extern void timer0_clear(void);     // clears timer count registers
extern uint16_t timer0_read(void);

// Timer1 control functions
#define timer1_start() TIMER1_ON = 1
#define timer1_stop() TIMER1_ON = 0
#define timer1_IF_clear() TIMER1_IF = 0
#define timer1_IF_read() TIMER1_IF
extern void timer1_clear(void);
extern uint16_t timer1_read(void);

// Timer2 control functions
extern void timer2_init(void);
#define timer2_start() TIMER2_ON = 1
#define timer2_stop() TIMER2_ON = 0
#define timer2_IF_clear() TIMER2_IF = 0
#define timer2_IF_read() TIMER2_IF
extern void timer2_clear(void);
extern uint16_t timer2_read(void);

// Timer3 control functions
#define timer3_start() TIMER3_ON = 1
#define timer3_stop() TIMER3_ON = 0
#define timer3_IF_clear() TIMER3_IF = 0
#define timer3_IF_read() TIMER3_IF
extern void timer3_clear(void);
extern uint16_t timer3_read(void);

// Timer4 control functions
#define timer4_start() TIMER4_ON = 1
#define timer4_stop() TIMER4_ON = 0
#define timer4_IF_clear() TIMER4_IF = 0
#define timer4_IF_read() TIMER4_IF
extern void timer4_clear(void);
extern uint16_t timer4_read(void);

// Timer5 control functions
#define timer5_start() TIMER5_ON = 1
#define timer5_stop() TIMER5_ON = 0
#define timer5_IF_clear() TIMER5_IF = 0
#define timer5_IF_read() TIMER5_IF
extern void timer5_clear(void);
extern uint16_t timer5_read(void);

// Timer6 control functions
#define timer6_start() TIMER6_ON = 1
#define timer6_stop() TIMER6_ON = 0
#define timer6_IF_clear() TIMER6_IF = 0
#define timer6_IF_read() TIMER6_IF
extern void timer6_clear(void);
extern uint16_t timer6_read(void);

#endif
