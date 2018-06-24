#include "../includes.h"

/* ************************************************************************** */
/*  Notes on the task system

    This is a minimalist background task scheduler that's designed to complement
    a superloop instead of replacing it.

    task_manager_update() needs to be called regularly in the main loop or
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
    eventCallback: a function pointer that event a task performs
    scheduledTime: the task will be performed when the current system time
                    equals this scheduled time
    repeat: if the task is to be repeated, it will be re-registered this number
            of system ticks into the future
*/
typedef struct {
    const char *name;
    task_callback_s eventCallback;
    system_time_t scheduledTime;
    uint16_t repeat;
} task_s;

/* ************************************************************************** */
// The maximum number of tasks appears to be limited by the PIC18's RAM page size.
// A page of RAM is 256 bytes, and a task object is 11 bytes long.
// Therefore, the maximum length of the queue is 11 * 23 = 253
#define MAX_NUM_OF_TASKS 25
#define FIRST_TASK 0

// This structure stores the event queue and related data.
struct{
    task_s queue[MAX_NUM_OF_TASKS];
    uint8_t numberOfTasks;
}tasks;

/* ************************************************************************** */
// task debug utilities

static void print_task(task_s *task)
{
    printf("task:(name:%s)(ptr:%p)(time:%u)(repeat:%u)\r\n", 
            task->name, task->eventCallback, task->scheduledTime, task->repeat);
}

static void print_task_queue(void)
{
    println("");
    println("-----");
    printf("Task queue contains: %d tasks\r\n", tasks.numberOfTasks);
    println("");

    if(tasks.numberOfTasks == 0) return;

    // step through the list and print each task
    for(uint8_t i = 0; i < tasks.numberOfTasks; i++)
    {
        print_task(&tasks.queue[i]);
        // every fifth task, add a line break
        if(((i + 1) % 5) == 0)
        {
            println("");
        }
    }

    println("-----");
    println("");
}

/* -------------------------------------------------------------------------- */
// Task queue manipulation utilities

#define queue_is_full() (tasks.numberOfTasks == MAX_NUM_OF_TASKS)
#define queue_is_empty() (tasks.numberOfTasks == 0)
#define copy_task_in_queue(source, destination) (tasks.queue[destination] = tasks.queue[source])
#define task_is_ready(taskIndex, currentTime) (tasks.queue[taskIndex].scheduledTime < currentTime)
#define get_task_scheduled_time(taskIndex) (tasks.queue[taskIndex].scheduledTime)

// Delete the contents of a given task, setting its elements to 'clean' values
static void task_clear(task_s *task)
{
    task->name = NULL;
    task->eventCallback = NULL;
    task->scheduledTime = UINT24_MAX;
    task->repeat = 0;
}

// swap two tasks
static void swap_tasks_in_queue(uint8_t taskA, uint8_t taskB)
{
    task_s copyOfTaskB = tasks.queue[taskB];
    tasks.queue[taskB] = tasks.queue[taskA];
    tasks.queue[taskA] = copyOfTaskB;
}

/* -------------------------------------------------------------------------- */

static void task_queue_sort(void)
{
    uint8_t currentTaskInQueue;

    int8_t indexOfLastSortedElement = -1;
    uint8_t currentElementBeingSwappedLeft;

    // If there aren't any tasks, then what are we doing here?
    if (queue_is_empty()) 
    {
        #if LOG_LEVEL_TASKS >= LOG_EVENTS
            println("Task Queue is empty!");
            println("Can't sort an empty list!");
        #endif

        return;
    }

    // The queue can't be out of order if there's only one task
    if (tasks.numberOfTasks == 1) 
    {
        #if LOG_LEVEL_TASKS >= LOG_EVENTS
            println("Task Queue only contains one task!");
            println("Can't sort a single item!");
        #endif

        return;
    }

    for (currentTaskInQueue = 0; currentTaskInQueue < tasks.numberOfTasks; currentTaskInQueue++){
        if (indexOfLastSortedElement == -1){
            indexOfLastSortedElement++;
        } else {
            currentElementBeingSwappedLeft = indexOfLastSortedElement+1;
            // while there's an element to the left of the current element, swap it left until it's in the correct place
            while (currentElementBeingSwappedLeft-1 > -1 && tasks.queue[currentElementBeingSwappedLeft].scheduledTime < tasks.queue[currentElementBeingSwappedLeft-1].scheduledTime){
                swap_tasks_in_queue(currentElementBeingSwappedLeft, currentElementBeingSwappedLeft-1);
                currentElementBeingSwappedLeft--;
            }
            indexOfLastSortedElement++;
        }
    }
}

// insert a task into the queue and then sort it
static void task_insert_and_sort(task_s *newTask)
{
    if(queue_is_full())
    {
        #if LOG_LEVEL_TASKS >= LOG_EVENTS
            println("task insertion failed, queue is full");
        #endif
        return;
    }

    tasks.queue[tasks.numberOfTasks] = *newTask;
    tasks.numberOfTasks++;

    task_queue_sort();
}

// remove a specified task from the queue and then sort what's left
static void task_remove(uint8_t taskID)
{
    // make sure the array isn't empty
    if(queue_is_empty())
    {
        #if LOG_LEVEL_TASKS >= LOG_EVENTS
            println("task removal failed, queue already empty");
        #endif
        return;
    }

    // Remove the task
    task_clear(&tasks.queue[taskID]);
    tasks.numberOfTasks--;

    // TODO: prove that this is shuffling properly
    // shuffle the array leftwards
    for(uint8_t i = taskID; i < tasks.numberOfTasks + 1; i++)
    {
        copy_task_in_queue(i + 1, i);
    }
}

/* ************************************************************************** */
// task system functions

void task_manager_init(void)
{
    tasks.numberOfTasks = 0;

    // clear task queue
    for(uint8_t i = 0; i < MAX_NUM_OF_TASKS; i++)
    {
        task_clear(&tasks.queue[i]);
    }
}

/* -------------------------------------------------------------------------- */

/*  task_manager_update() should be called from application code at regular
    intervals, during periods where no time-critical work is being done.

    The project this task manager was designed for has a series of nested state
    machine loops that are used to poll user input and respond.
*/
void task_manager_update(void)
{
    if(queue_is_empty()) return;

    // return early if the first task isn't ready yet
    system_time_t currentTime = systick_read();
    if(!task_is_ready(FIRST_TASK, currentTime)) return;

    #if LOG_LEVEL_TASKS >= LOG_EVENTS
        printf("task is ready @%u\r\n", currentTime);
        print("executing");
        print_task(&tasks.queue[FIRST_TASK]);
    #endif

    // Make sure we don't execute a null function pointer
    if(tasks.queue[FIRST_TASK].eventCallback != NULL){
        // execute it
        tasks.queue[FIRST_TASK].eventCallback();

        // if the task should be repeated, re-register it
        if(tasks.queue[FIRST_TASK].repeat != 0)
        {
            // grab a copy of the current task
            task_s repeatingTask = tasks.queue[FIRST_TASK];
            // update its scheduledTime
            repeatingTask.scheduledTime = currentTime + repeatingTask.repeat;
            // throw it back in the queue
            task_insert_and_sort(&repeatingTask);
        }        
    } else { // pointer is null, do not execute
        #if LOG_LEVEL_TASKS >= LOG_ERROR
            println(">>> NULL POINTER EXCEPTION <<< ");
        #endif
        // while(1); // trap
    }

    task_remove(FIRST_TASK);
}

/* -------------------------------------------------------------------------- */

// create a task object and add it to the task queue
int8_t task_register(const char *name, task_callback_s callback, 
                     system_time_t time, uint16_t repeat)
{
    if(queue_is_full()) return -1;

    // create and init a new task object
    task_s newTask;
    task_clear(&newTask);

    // populate the new task
    newTask.name = name;
    newTask.eventCallback = callback;
    newTask.scheduledTime = time;
    newTask.repeat = repeat;

    #if LOG_LEVEL_TASKS >= LOG_EVENTS
        print("Registering ");
        print_task(&tasks.queue[FIRST_TASK]);
    #endif

    // add it to the queue
    task_insert_and_sort(&newTask);

    return 0;
}

/* -------------------------------------------------------------------------- */

// remove a task from the queue using its name as a key
int8_t task_deregister(const char *name)
{
    int8_t index = task_queue_lookup(name);

    // the indicated task was not found
    if (index == -1) return -1;

    // remove the task
	task_remove(index);
    
    return 0;
}

// check if a task with the given name exists in the queue
int8_t task_queue_lookup(const char *name)
{
    for(uint8_t i = 0; i < MAX_NUM_OF_TASKS; i++) {
		if (!strcmp(name, tasks.queue[i].name)) {
			return 0; // found a match
		}
    }
    return -1; // didn't find a match
}

/* ************************************************************************** */
/*  Task Manager testing utilities

*/
// dummy tasks to be used as callbacks in task_self_test()
void task_beep(void)
{
    printf("beep %d\r\n", systick_read());
}

void task_boop(void)
{
    printf("boop %d\r\n", systick_read());
}

void task_fizz(void)
{
    printf("fizz %d\r\n", systick_read());
}

void task_buzz(void)
{
    printf("buzz %d\r\n", systick_read());
}

void task_dummy(void)
{
    printf("time: %d\r\n", systick_read());
}
/* ************************************************************************** */

// print out the size in bytes of the various objects used in the queue
void print_object_sizes(void)
{
    println("");
    println("=====");
    println("Check size of task queue objects");
    printf("sizeof tasks: %d bytes\r\n", sizeof(tasks));
    printf("sizeof tasks: %d bytes\r\n", sizeof(tasks.queue));
    printf("sizeof tasks: %d bytes\r\n", sizeof(tasks.queue[0]));
}

// print the values of the function pointers to cross-reference against other output
void print_pointer_values(void)
{
    println("");
    println("=====");
    println("Check function pointer values");
    printf("task_beep: %p\r\n", task_beep);
    printf("task_boop: %p\r\n", task_boop);
    printf("task_fizz: %p\r\n", task_fizz);
    printf("task_buzz: %p\r\n", task_buzz);
}

const system_time_t random_times[] = {
    6750, 5698, 8424, 3556, 1678, 6438, 8957, 586, 600, 531, // 10 items
    4511, 106, 3014, 1309, 3065, 4848, 8870, 244, 3183, 3777, // 20 items
    2675, 6938, 6995, 1841, 1171, 6554, 7143, 5970, 7052, 1622, // 30 items
    // 7810, 2342, 7783, 4372, 8031, 9432, 6134, 3114, 3247, 4294, // 40 items
    // 2768, 138, 3279, 4376, 7459, 3443, 1670, 7893, 3908, 1025, // 50 items
    // 8667, 9322, 8222, 9365, 8866, 9266, 3125, 6630, 6584, 6697, // 60 items
    // 1683, 8651, 9668, 9218, 9391, 299, 906, 8828, 2042, 609 // 70 items
};

void task_queue_fill_test(void)
{
    println("");
    println("=====");
    println("Task setup");

    uint8_t i = 0;

    // fill up that queue
    task_register("1", task_dummy, random_times[i++], 0);
    task_register("2", task_dummy, random_times[i++], 0);
    task_register("3", task_dummy, random_times[i++], 0);
    task_register("4", task_dummy, random_times[i++], 0);
    task_register("5", task_dummy, random_times[i++], 0);

    task_register("6", task_dummy, random_times[i++], 0);
    task_register("7", task_dummy, random_times[i++], 0);
    task_register("8", task_dummy, random_times[i++], 0);
    task_register("9", task_dummy, random_times[i++], 0);
    task_register("10", task_dummy, random_times[i++], 0);

    task_register("11", task_dummy, random_times[i++], 0);
    task_register("12", task_dummy, random_times[i++], 0);
    task_register("13", task_dummy, random_times[i++], 0);
    task_register("14", task_dummy, random_times[i++], 0);
    task_register("15", task_dummy, random_times[i++], 0);

    task_register("16", task_dummy, random_times[i++], 0);
    task_register("17", task_dummy, random_times[i++], 0);
    task_register("18", task_dummy, random_times[i++], 0);
    task_register("19", task_dummy, random_times[i++], 0);
    task_register("20", task_dummy, random_times[i++], 0);

    task_register("21", task_dummy, random_times[i++], 0);
    task_register("22", task_dummy, random_times[i++], 0);
    task_register("23", task_dummy, random_times[i++], 0);
    task_register("24", task_dummy, random_times[i++], 0);
    task_register("25", task_dummy, random_times[i++], 0);

    // task_register("26", task_dummy, random_times[i++], 0);
    // task_register("27", task_dummy, random_times[i++], 0);
    // task_register("28", task_dummy, random_times[i++], 0);
    // task_register("29", task_dummy, random_times[i++], 0);
    // task_register("30", task_dummy, random_times[i++], 0);

    // task_register("31", task_dummy, random_times[i++], 0);
    // task_register("32", task_dummy, random_times[i++], 0);
    // task_register("33", task_dummy, random_times[i++], 0);
    // task_register("34", task_dummy, random_times[i++], 0);
    // task_register("35", task_dummy, random_times[i++], 0);

    println("");
    println("=====");
    println("Confirm task queue is sorted");
    print_task_queue();
}

void task_sorting_simple_test(void)
{
    println("");
    println("=====");
    println("Task setup");
    task_register("fizz", task_fizz, 3000, 0);
    task_register("buzz", task_buzz, 4000, 0);
    task_register("boop", task_boop, 2000, 0);
    task_register("beep", task_beep, 1000, 0);

    println("");
    println("=====");
    println("Confirm task queue is sorted");
    print_task_queue();
}

void task_sorting_stress_test(void)
{
    
}

void task_manager_self_test(void)
{
    println("");

    // print_object_sizes();
    
    // print_pointer_values();

    task_queue_fill_test();

    // task_sorting_simple_test();

    // task_sorting_stress_test();
}