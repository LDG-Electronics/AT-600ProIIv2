#include "includes.h"
#include "tasks.h"

/* ************************************************************************** */
/*  Notes on the task system

    This is a minimalist task scheduler that's designed to complement a
    superloop instead of replacing it.




*/
/* ************************************************************************** */

#define MAX_NUM_OF_TASKS 32

struct{
    task_s queue[MAX_NUM_OF_TASKS];
    uint8_t firstTask;
    uint8_t numberOfTasks;
}tasks;

task_s test_task;

/* ************************************************************************** */
// task manipulation utilities

static void task_clear(task_s *task)
{
    task->event_callback = NULL;
    task->scheduled_time = 0;
    task->repeat = 0;
    task->nextTask = 0;
    task->prevTask = 0;
}

static void task_raw_insert(task_s *task, uint8_t index)
{
    tasks.queue[index].event_callback = NULL;
    tasks.queue[index].scheduled_time = 0;
    tasks.queue[index].repeat = 0;
    tasks.queue[index].nextTask = 0;
    tasks.queue[index].prevTask = 0;
}
/*  task_insert_sorted() inserts a new task into the queue at the correct location

*/
static void task_insert_sorted(task_s *newTask)
{
    uint8_t newID = 0;
    uint8_t nextID = tasks.firstTask;
    uint8_t prevID = 0;

    // identify an empty slot in the queue
    for(newID = 0; newID < MAX_NUM_OF_TASKS; newID++)
    {
        if(tasks.queue[newID].scheduled_time == 0) break;
    }

    // copy newTask into the empty slot
    tasks.queue[newID] = *newTask;

    // step through the list and find where newTask belongs
    for(uint8_t i = 0; i < tasks.numberOfTasks; i++)
    {
        if(tasks.queue[nextID].scheduled_time < tasks.queue[newID].scheduled_time)
        {
            // if newTask is sooner than the next task, then newTask will be
            // inserted before the next task
            break;
        }
        // step to the next task in the queue
        nextID = tasks.queue[nextID].nextTask;
    }

    if (nextID == tasks.firstTask) {
        // newTask is the new firstTask

        // link the old first task to the new task
        tasks.queue[tasks.firstTask].prevTask = newID;

        // link the new task to the old first task
        tasks.queue[newID].nextTask = nextID;

        // mark the new first task
        tasks.firstTask = newID;
    } else {
        // newTask is in the middle of the list

        // identify the previous 
        prevID = tasks.queue[nextID].prevTask;

        // update the new tasks pointers
        tasks.queue[newID].nextTask = nextID;
        tasks.queue[newID].prevTask = prevID;
        // update the neighbor's pointers
        tasks.queue[prevID].nextTask = newID;
        tasks.queue[nextID].prevTask = newID;
    }

    tasks.numberOfTasks++;
}

static void task_remove(uint8_t taskID)
{
    uint8_t nextID = 0;
    uint8_t prevID = 0;

    if(taskID == tasks.firstTask) {
        // the task to be removed is the first one on the list
        nextID = tasks.queue[taskID].nextTask;
        
        tasks.firstTask = nextID;
        // update the neighbor's pointers
        tasks.queue[nextID].prevTask = 0;
    } else {
        // the task to be removed is in the middle of the list

        // identify the neighbors of the task to be removed
        nextID = tasks.queue[taskID].nextTask;
        prevID = tasks.queue[taskID].prevTask;

        // update the neighbor's pointers
        tasks.queue[prevID].nextTask = nextID;
        tasks.queue[nextID].prevTask = prevID;
    }

    task_clear(&tasks.queue[taskID]);

    tasks.numberOfTasks--;
}

/* ************************************************************************** */
// task system functions

void tasks_init(void)
{
    tasks.firstTask = 0;

    for(uint8_t i = 0; i < MAX_NUM_OF_TASKS; i++)
    {
        task_clear(&tasks.queue[i]);
    }

    task_clear(&test_task);
}

int8_t task_register(task_callback_s callback, uint24_t time, uint8_t repeat)
{
    uint8_t nextID = 0;
    uint8_t prevID = 0;

    // check that the queue isn't full
    if(tasks.numberOfTasks == MAX_NUM_OF_TASKS) return -1;

    // create and init a new task object
    task_s new_task;
    task_clear(&new_task);

    task_insert_sorted(&new_task);

    return 0;
}

void task_manager(void)
{
    // read current time
    uint24_t current_time = systick_read();
    
    // return early if the first task isn't ready yet
    if (tasks.queue[tasks.firstTask].scheduled_time < current_time) return;

    // execute the task
    tasks.queue[tasks.firstTask].event_callback();

    // remove the task from the list
    task_remove(tasks.firstTask);
}
