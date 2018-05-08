#ifndef _TASKS_H_
#define _TASKS_H_

/* ************************************************************************** */

// Function signature for a task callback function, to be implemented by the
// caller of this library
typedef void (*task_callback_s) (void);

/* ************************************************************************** */

extern void tasks_init(void);

extern int8_t task_register(char *name, task_callback_s callback, 
                            uint24_t time, uint16_t repeat);

extern void task_manager_update(void);

#endif