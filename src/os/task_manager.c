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
    uint24_t scheduledTime;
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

// check to see if there's a task to the left of the given task
static int8_t task_to_left_exists(uint8_t indexOfTaskInQueue){
    if(indexOfTaskInQueue == 0){
        return false;
    } else {
        return true;
    }
}

static int8_t scheduled_time_of_task_in_queue(uint8_t indexOfTaskInQueue){
    return tasks.queue[indexOfTaskInQueue].scheduledTime;
}

/* -------------------------------------------------------------------------- */

// ::::sort the task queue in ascending order of scheduledTime::::

// This sort takes the array of tasks and assumes they're all unsorted.
// It then grows a group of tasks from the left of the array (index zero) to the right
// until it encompasses every valid task in the array. It does this by
// adding one task at a time to the group. 

// Here's an example:
// [] is an empty element in the array.
// [*] is an element in the array with a task in it.
// [g] is an element in the array with a task in it that's been added to the group.

// Step 1: Nothing's in the group.
// [*][*][*][*][*][*][*][][][][][][][]
// Step 2: One element's in the group.
// [g][*][*][*][*][*][*][][][][][][][]
// Step 3: Two elements are in the group.
// [g][g][*][*][*][*][*][][][][][][][]
// et cetera, until all elements with tasks are in the group
// [g][g][g][g][g][g][g][][][][][][][]

// But the tasks aren't added blindly. As they're added,
// they're moved within the group so that they're in ascending order, based on each task's scheduledTime.
// To do this, the task being added is swapped from it's initial position to the right of the group into it's correct position in the group.

// Here's an example:
// [] is an empty element in the array.
// [*] is an element in the array with a task in it.
// [g] is an element in the array with a task in it that's been added to the group.
// [s] is an element in the array with a task in it that's been added to the group and moved (sorted) into its proper position.

// Step 1: Nothing's in the group.
// [*][*][*][*][*][*][*][][][][][][][]
// Step 2: One element's added to the group.
// [g][*][*][*][*][*][*][][][][][][][]
// Step 3: Since there's only one element, that element is in the correct position. The group is sorted.
// [s][*][*][*][*][*][*][][][][][][][]
// Step 4: Two elements are in the group. One element is sorted, the other is being added.
// [s][g][*][*][*][*][*][][][][][][][]
// Step 4: The new element is either swapped with the old element or it stays in place. In either case, the new task in the group is moved to the position it needs to be in for the array to be sorted.
// [s][s][*][*][*][*][*][][][][][][][]
// et cetera, until all elements with tasks are in the group

// For clarity, let's look at what a typical case might look like midway through the sort.
// [s] [s] [s] [s] [*][*][*][][][][][][][]
// Let's assume that the group members have the following scheduledTimes.
// [30][50][70][90][*][*][*][][][][][][][]
// And that the latest addition to the group has a scheduledTime of 40.  
// [s] [s] [s] [s] [g] [*][*][][][][][][][]
// [30][50][70][90][40][*][*][][][][][][][]
// The new group member will be moved to the left until it's in the correct place.
// [30][50][70][90][40][*][*][][][][][][][]
// [30][50][70][40][90][*][*][][][][][][][]
// [30][50][40][70][90][*][*][][][][][][][]
// [30][40][50][70][90][*][*][][][][][][][]
// Now every task in the group is in order.
// [s] [s] [s] [s] [s] [*][*][][][][][][][]
// [30][40][50][70][90][*][*][][][][][][][]
static void ians_insertion_sort(void)
{
    int8_t indexOfLastTaskInSortedGroup = -1;
    uint8_t currentTaskInQueue;
    uint8_t indexOfTaskBeingAddedToSortedGroup;

    int8_t indexOfLastSortedElement = -1;
    uint8_t currentElementBeingSwappedLeft;

    // If there aren't any tasks, then what are we doing here?
    if (queue_is_empty()) 
    {
        println("Task Queue is empty!");
        println("Can't sort an empty list!");
        return;
    }

    // The queue can't be out of order if there's only one task
    if (tasks.numberOfTasks == 1) return;

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

    // TODO: Ian fix your shit

    // // Go through each existing task in the queue...
    // for (currentTaskInQueue = 0; currentTaskInQueue < tasks.numberOfTasks; currentTaskInQueue++){
    //     // if it's the first task, then leave it in place and call it sorted
    //     if (indexOfLastTaskInSortedGroup == -1){
    //         indexOfLastTaskInSortedGroup++;
    //     } else { // if it's not the first task...
    //         taskBeingAdded = indexOfLastTaskInSortedGroup+1;
    //         // ...while there's a task to the left of the current task, check to see if the current task needs to be swapped with the task to its left
    //         // if it does, swap it. Continue until it doesn't.
    //         // if it doesn't, the the task is in its proper position and the group is sorted
    //         while (task_to_left_exists(indexOfTaskBeingAddedToSortedGroup) && scheduled_time_of_task_in_queue(indexOfTaskBeingAddedToSortedGroup) < scheduled_time_of_task_in_queue(indexOfTaskBeingAddedToSortedGroup-1)){
    //             swap_tasks_in_queue(indexOfTaskBeingAddedToSortedGroup, indexOfTaskBeingAddedToSortedGroup-1);
    //             indexOfTaskBeingAddedToSortedGroup--;
    //         }
    //         indexOfLastTaskInSortedGroup++;
    //     }
    // }

void shell_sort(void)
{

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

    ians_insertion_sort();
}

// remove a specified task from the queue and then sort it
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

// create a task object and add it to the task queue
int8_t task_register(const char *name, task_callback_s callback, uint24_t time, uint16_t repeat)
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

// remove a task from the queue using its name as a key
int8_t task_deregister(const char *name)
{
    for(uint8_t i = 0; i < MAX_NUM_OF_TASKS; i++) {
		if (!strcmp(name, tasks.queue[i].name)) {
			task_remove(i);
            return 0;
		}
    }
    return -1; // didn't deregister a task
}

/* -------------------------------------------------------------------------- */

// This function should be called periodically to monitor the task queue and
// execute any tasks that are ready
void task_manager_update(void)
{
    if(queue_is_empty()) return;

    // return early if the first task isn't ready yet
    uint24_t currentTime = systick_read();
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

/* ************************************************************************** */
// Task Manager testing utilities

// dummy tasks to be used as callbacks in task_self_test()
void task_beep(void)
{
    uint24_t currentTime = systick_read();
    printf("beep %d\r\n", currentTime);
}

void task_boop(void)
{
    uint24_t currentTime = systick_read();
    printf("boop %d\r\n", currentTime);
}

void task_fizz(void)
{
    uint24_t currentTime = systick_read();
    printf("fizz %d\r\n", currentTime);
}

void task_buzz(void)
{
    uint24_t currentTime = systick_read();
    printf("buzz %d\r\n", currentTime);
}

void task_dummy(void)
{
    uint24_t currentTime = systick_read();
    // printf("time: %d\r\n", currentTime);
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

void task_queue_fill_test(void)
{
    println("");
    println("=====");
    println("Task setup");

    // fill up that queue
    task_register("1", task_dummy, 11000, 0);
    task_register("2", task_dummy, 10000, 0);
    task_register("3", task_dummy, 12000, 0);
    task_register("4", task_dummy, 13000, 0);
    task_register("5", task_dummy, 7000, 0);

    task_register("6", task_dummy, 5000, 0);
    task_register("7", task_dummy, 9000, 0);
    task_register("8", task_dummy, 5000, 0);
    task_register("9", task_dummy, 5000, 0);
    task_register("10", task_dummy, 5000, 0);

    task_register("11", task_dummy, 8000, 0);
    task_register("12", task_dummy, 30000, 0);
    task_register("13", task_dummy, 5000, 0);
    task_register("14", task_dummy, 10500, 0);
    task_register("15", task_dummy, 6000, 0);

    task_register("16", task_dummy, 15000, 0);
    task_register("17", task_dummy, 20000, 0);
    task_register("18", task_dummy, 10000, 0);
    task_register("19", task_dummy, 1000, 0);
    task_register("20", task_dummy, 1000, 0);

    task_register("21", task_dummy, 1000, 0);
    task_register("22", task_dummy, 1000, 0);
    task_register("23", task_dummy, 1000, 0);
    task_register("24", task_dummy, 1000, 0);
    task_register("25", task_dummy, 1000, 0);

    // task_register("26", task_dummy, 1000, 0);
    // task_register("27", task_dummy, 1000, 0);
    // task_register("28", task_dummy, 1000, 0);
    // task_register("29", task_dummy, 1000, 0);
    // task_register("30", task_dummy, 1000, 0);

    // task_register("31", task_dummy, 1000, 0);
    // task_register("32", task_dummy, 1000, 0);

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