#ifndef _TASKS_H_
#define _TASKS_H_

/* ************************************************************************** */

typedef void (*task_callback_s) (void);

typedef struct {
    char *name;
    task_callback_s event_callback;
    uint24_t scheduled_time;
    uint16_t repeat;
    uint8_t nextTask;
    uint8_t prevTask;
} task_s;

/* ************************************************************************** */

extern void tasks_init(void);

extern int8_t task_register(char *name, task_callback_s callback, 
                            uint24_t time, uint16_t repeat);

extern void task_manager_update(void);

#endif