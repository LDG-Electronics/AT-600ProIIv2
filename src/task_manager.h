#ifndef _TASKS_H_
#define _TASKS_H_

/* ************************************************************************** */
/*  Notes on the Task Manager subsystem

    task_manager_update() needs to be called periodically from the 'user' code.
    
    The Task Manager supports repeat

    A task consists of 4 pieces of data:
    1: name - a string to identify the task
    2: callback - a function to called when the specified time has been reached
    3: time - when the task callback should be executed
    4: repeat - whether the task should be re-registered after execution

/* ************************************************************************** */

// Function signature for a task callback function, to be implemented by the
// caller of this library
typedef void (*task_callback_s) (void);

/* ************************************************************************** */
// setup
extern void task_manager_init(void);

// Add a task to the queue
extern int8_t task_register(const char *name, task_callback_s callback, 
                            uint24_t time, uint16_t repeat);

// Remove a task from the queue
extern int8_t task_deregister(const char *name);

// Check if the specified task exists in the queue
extern int8_t task_queue_lookup(const char *name);

// This function should be called periodically to monitor the task queue and
// execute any tasks that are ready
extern void task_manager_update(void);

/* -------------------------------------------------------------------------- */

extern void task_manager_self_test(void);

#endif