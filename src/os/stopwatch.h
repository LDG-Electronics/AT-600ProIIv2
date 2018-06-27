#ifndef _STOPWATCH_H_
#define _STOPWATCH_H_

/* ************************************************************************** */

/*  Notes on using stopwatch module:
    
    Measuring a time longer than 1 overflow of Timer0 is going to involve
    several interrupt triggers.  Be careful using either stopwatch inside
    critical sections.
    
    Some care must be taken to prevent multiple uses of the stopwatch from
    overlapping. Timers are a limited resource, and obviously cannot be used
    for multiple tasks at once. Any call to xx_stopwatch_begin() will overwrite
    any active stopwatches, corrupting any possible readings. 
    
    There is currently no mechanism in place to detect or avoid these 
    conditions, since the stopwatch is NOT intended for use in production code.
*/

/* -------------------------------------------------------------------------- */

// Setup
extern void stopwatch_init(void);

// Microsecond stopwatch functions, prints elapsed time in us +/- 1%
extern void us_stopwatch_begin(void);
extern void us_stopwatch_end(void);

#define us_stopwatch_pause() timer0_stop()
#define us_stopwatch_resume() timer0_start()

// Millisecond stopwatch functions, prints elapsed time in ms +/- 1ms
extern void ms_stopwatch_begin(void);
extern void ms_stopwatch_end(void);

/* -------------------------------------------------------------------------- */

// Test function used to calibrate the stopwatch
extern void stopwatch_and_delay_test(void);

#endif // _STOPWATCH_H_