#ifndef _EVENT_SCHEDULER_H_
#define _EVENT_SCHEDULER_H_

/* ************************************************************************** */
/*  Notes on the Event Scheduler subsystem

    event_scheduler_update() needs to be called periodically from the 'user'
code.

    A event consists of 4 pieces of data:
    1: name - a string to identify the event
    2: callback - a function to called when the specified time has been reached
    3: time - when the event callback should be executed
    4: repeat - whether the event should be re-registered after execution

/* ************************************************************************** */

/*  Notes on Events:

    In this system, a event is the function that is called when the designated
    time has been reached.

    Event best practices:

    An Event should:
        contain minimal side effects
        run to completion with no chance to stall or hang
        complete in a predictable amount of time

    An Event should NOT:
        contain blocking delays
        perform any polling operations
        manipulate any global state(?)

    An ideal event is short, simple, and deterministic.

    TODO: add event examples
    Checking a sensor and updating a display with the information, but taking no
    other action is a good example of a event.
*/
typedef void (*event_callback_t)(void);
/* ************************************************************************** */

// setup
extern void event_scheduler_init(void);

/*  event_scheduler_update() should be called from application code at regular
    intervals, during periods where no time-critical work is being done.

    The project this event manager was designed for has a series of nested state
    machine loops that are used to poll user input and respond.
*/
extern void event_scheduler_update(void);

// Add an event to the queue
extern int8_t event_register(const char *name, event_callback_t callback,
                             system_time_t time, uint16_t repeat);

// Remove an event from the queue
extern int8_t event_deregister(const char *name);

// Check if the specified event exists in the queue
extern int8_t event_queue_lookup(const char *name);

/* -------------------------------------------------------------------------- */
/*  event_manager_self_test() is a suite of tests that exercises the event
    manager system by created a series of dummy events, adding them to the event
    queue, and checking that the queue is properly sorted.

*/
extern void event_manager_self_test(void);

#endif