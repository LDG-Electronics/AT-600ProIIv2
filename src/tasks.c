#include "includes.h"
#include "tasks.h"

/* ************************************************************************** */
/*  Notes on the task system

    This is a minimalist task scheduler that's designed to complement a
    superloop instead of replacing it.

    task_manager_update() needs to be called regularly in the mainloop or
    equivalent structure. Any possible event callback should be relatively
    short, contain no majors delays, and should cause limited side-effects.
    
    Timing is not particularly strict in this scheduler, especially because it
    allows the main program to block at will. This is desirable behavior in many
    embedded devices.

*/

/* ************************************************************************** */
/*  This structure stores the information related to a single task.

    The fields are:
    name: a string that identifies the task
    event_callback: a function pointer that event a task performs
    scheduled_time: the task will be performed when the current system time
                    equals this scheduled time
    repeat: if the task is to be repeated, it will be re-registered this number
            of system ticks into the future
    nextTask and prevTask: indexes used to refer to tasks neighbors in the
                           doubly-linked-list
*/
typedef struct {
    char *name;
    task_callback_s event_callback;
    uint24_t scheduled_time;
    uint16_t repeat;
    uint8_t nextTask;
    uint8_t prevTask;
} task_s;

/* ************************************************************************** */
/*  This structure stores the event queue and related data.

    The event queue is a doubly-linked-list, but using array indexes instead of
    pointers. This keeps us from needing dynamic memory allocation like a 
    normal linked list, and SHOULD allow faster lookups than actual pointers,
    considering the 8bit system this is designed for.

*/
#define MAX_NUM_OF_TASKS 32

struct{
    task_s queue[MAX_NUM_OF_TASKS];
    uint8_t firstTask;
    uint8_t numberOfTasks;
}tasks;

/* ************************************************************************** */
// task manipulation utilities

void print_task(task_s *task)
{
    printf("task name: %s", task->name);
}

void print_task_queue(void)
{
    #if LOG_LEVEL_TASKS >= LOG_EVENTS

    println("-----------------------------------------------");
	println("Task Queue Debug");
    println("");

    printf("queue contains: %d tasks", tasks.numberOfTasks);

    if(tasks.numberOfTasks == 0) return;

    uint8_t nextID = tasks.firstTask;

    // step through the list and print each task
    for(uint8_t i = 0; i < tasks.numberOfTasks; i++)
    {
        printf("Task #%d: %s", i, tasks.queue[nextID].name);
        // step to the next task in the queue
        nextID = tasks.queue[nextID].nextTask;
    }

    #endif
}

/* -------------------------------------------------------------------------- */

/*  task_clear() initializes a new task_s object with blank default values

*/
static void task_clear(task_s *task)
{
    task->name = NULL;
    task->event_callback = NULL;
    task->scheduled_time = 0;
    task->repeat = 0;
    task->nextTask = 0;
    task->prevTask = 0;
}

/*  task_insert_sorted() inserts a new task into the queue at the correct location

*/
static void task_sorted_insert(task_s *newTask)
{   
    // identify an empty slot in the queue
    uint8_t newID; // location of new task
    for(newID = 0; newID < MAX_NUM_OF_TASKS; newID++)
    {
        if(tasks.queue[newID].scheduled_time == 0) break;
    }

    // copy newTask into the empty slot
    tasks.queue[newID] = *newTask;

    #if LOG_LEVEL_TASKS >= LOG_EVENTS
        printf("empty slot found at %d\r\n", newID);
    #endif

    uint8_t nextID = tasks.firstTask;
    uint8_t prevID = 0;
    
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

    #if LOG_LEVEL_TASKS >= LOG_EVENTS
        printf("newTask will be inserted before task %d\r\n", nextID);
    #endif

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

    #if LOG_LEVEL_TASKS >= LOG_EVENTS
        printf("the queue now contains %d tasks\r\n", tasks.numberOfTasks);
    #endif
}

/*  task_remove() removes a specified task from the queue, and updates its
    neighbors nextTask and prevTask pointers

*/
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
    tasks.numberOfTasks = 0;

    // clear task queue
    for(uint8_t i = 0; i < MAX_NUM_OF_TASKS; i++)
    {
        task_clear(&tasks.queue[i]);
    }
}

int8_t task_register(char *name, task_callback_s callback, 
                     uint24_t time, uint16_t repeat)
{
    uint8_t nextID = 0;
    uint8_t prevID = 0;

    // check that the queue isn't full
    if(tasks.numberOfTasks == MAX_NUM_OF_TASKS) return -1;

    // create and init a new task object
    task_s new_task;
    task_clear(&new_task);

    // populate the new task
    new_task.name = name;
    new_task.event_callback = callback;
    new_task.scheduled_time = time;
    new_task.repeat = repeat;

    // add it to the queue
    task_sorted_insert(&new_task);

    return 0;
}

void task_manager_update(void)
{
    // return early if the queue is empty
    if(tasks.numberOfTasks == 0) return;

    // return early if the first task isn't ready yet
    uint24_t current_time = systick_read();
    if(tasks.queue[tasks.firstTask].scheduled_time < current_time) return;

    // grab a copy of the current task
    task_s currentTask = tasks.queue[tasks.firstTask];
    
    // remove the task from the list
    task_remove(tasks.firstTask);

    // execute it
    currentTask.event_callback();

    // if the task should be repeated, re-register it
    if(currentTask.repeat != 0)
    {
        currentTask.scheduled_time = current_time + currentTask.repeat;
        task_sorted_insert(&currentTask);
    }
}
