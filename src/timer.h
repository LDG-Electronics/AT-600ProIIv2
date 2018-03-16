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
#define TIMER1_H_REG TMR1H
#define TIMER1_L_REG TMR1L
#define TIMER2_REG TMR2
#define TIMER3_H_REG TMR3H
#define TIMER3_L_REG TMR3L

/* ************************************************************************** */

// Timer0 is unused
// Timer1 is used in the frequency counter.  Not free-running.
// Timer2 is unused
// Timer3 is unused
// Timer4 is used for the millisecond delay function.  Not free-running.
// Timer5 is unused
// Timer6 is used in serial communication.  Free-running.

/* ************************************************************************** */

// Setup and control function for hardware timers.
// Each timerX_init() and timerX_clear() can be customized for desired timer duration.

// Any additional or specialized timer functions should follow the existing
// naming convention of:  timerX_verb()

// Timer0 control functions
extern void timer0_init(void);      // sets up timer with desired settings
extern void timer0_start(void);     // sets timerX enable bit
extern void timer0_stop(void);      // clears timerX enable bit
extern void timer0_clear(void);     // clears timer count registers
extern void timer0_reset(void);     // stops timer, clears count,
                                    // clears interrupt flag, starts timer
extern uint16_t timer0_read(void);

// Timer1 control functions
extern void timer1_init(void);
extern void timer1_start(void);
extern void timer1_stop(void);
extern void timer1_clear(void);
extern uint16_t timer1_read(void);

// Timer2 control functions
extern void timer2_init(void);
extern void timer2_start(void);
extern void timer2_stop(void);
extern void timer2_clear(void);
extern uint16_t timer2_read(void);

// Timer3 control functions
extern void timer3_init(void);
extern void timer3_start(void);
extern void timer3_stop(void);
extern void timer3_clear(void);
extern void timer3_reset(void);
extern uint16_t timer3_read(void);

// Timer4 control functions
extern void timer4_init(void);
extern void timer4_start(void);
extern void timer4_stop(void);
extern void timer4_clear(void);
extern uint16_t timer4_read(void);

// Timer5 control functions
extern void timer5_init(void);
extern void timer5_start(void);
extern void timer5_stop(void);
extern void timer5_clear(void);
extern uint16_t timer5_read(void);

// Timer6 control functions
extern void timer6_init(void);
extern void timer6_start(void);
extern void timer6_stop(void);
extern void timer6_clear(void);
extern uint16_t timer6_read(void);

#endif /* _TIMER_H_ */
