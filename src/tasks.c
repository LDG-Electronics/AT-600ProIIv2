#include "includes.h"

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
    event_callback: a function pointer that event a task performs
    scheduledTime: the task will be performed when the current system time
                    equals this scheduled time
    repeat: if the task is to be repeated, it will be re-registered this number
            of system ticks into the future
*/
typedef struct {
    char *name;
    task_callback_s event_callback;
    uint24_t scheduledTime;
    uint16_t repeat;
} task_s;

/* ************************************************************************** */
/*  This structure stores the event queue and related data.


*/
#define MAX_NUM_OF_TASKS 32
#define FIRST_TASK 0

struct{
    task_s queue[MAX_NUM_OF_TASKS];
    uint8_t numberOfTasks;
}tasks;

/* ************************************************************************** */
// task manipulation utilities

void print_task(task_s *task)
{
    printf("task:(%s@%d)\r\n", task->name, task->scheduledTime);
}

void print_task_queue(void)
{
    #if LOG_LEVEL_TASKS >= LOG_EVENTS

    println("");
    println("-----");
    printf("Task queue contains: %d tasks\r\n", tasks.numberOfTasks);
    println("");

    if(tasks.numberOfTasks == 0) return;

    // step through the list and print each task
    for(uint8_t i = 0; i < tasks.numberOfTasks; i++)
    {
        print_task(&tasks.queue[i]);
    }

    println("-----");
    println("");

    #endif
}

/* -------------------------------------------------------------------------- */

/*  task_clear() initializes a task_s object with blank default values

*/
static void task_clear(task_s *task)
{
    task->name = NULL;
    task->event_callback = NULL;
    task->scheduledTime = UINT24_MAX;
    task->repeat = 0;
}


// sort the task queue in ascending order of scheduledTime

static void swap_tasks_in_queue(uint8_t source, uint8_t destination){
    task_s tempCopyOfTask;
    tempCopyOfTask = tasks.queue[destination];
    tasks.queue[destination] = tasks.queue[source];
    tasks.queue[source] = tempCopyOfTask;
}


static void ians_insertion_sort(){
    int8_t indexOfLastSortedElement = -1;
    uint8_t currentTaskInQueue;
    uint8_t currentElementBeingSwappedLeft;
    uint8_t currentSortedElement;
    task_s tempCopyOfTask;

    // us_stopwatch_begin();

    if (tasks.numberOfTasks == 1){ // Todo: explain why this is weird.
        return;
    }

    // Go through each existing task in the queue...
    for (currentTaskInQueue = 0; currentTaskInQueue < tasks.numberOfTasks; currentTaskInQueue++){
        // ...and compare it to all of the tasks that are going to be in the group of tasks that are sorted already.
        // Obviously this group won't have anything in it to start of with, but it will grow over time.
        // As it grows, we'll store them to the left of the array - closer to index zero. And we'll keep track of it with indexOfLastSortedElement.
        // So up to and including the indexOfLastSortedElement, everything's sorted.
        
        // Here we go.
        // We'll create a variable called currentSortedElement to keep track of which of the sorted elements we're working with.
        // We'll also use the indexOftheLastSortedElement to know where to start comparing. We're starting with indexOfTheLastSortedElement because...
        // ...we want to figure out where in the group of sorted elements we should stick the unsorted thing we're working with - the currentTaskInQueue that we're looking at.
        // So we'll go from right to left through the sorted elements and compare each element to the element we're inserting to see which is smaller.
        // If the element we're inserting is smaller, then we need to keep going through the array to the left until we find it isn't (or we get to the front of the array).
        // Then we'll know that we're in the right place to insert the element.
        // But wait - where will we put it!
        // To ensure that there's a place to put it, we're going to move the sorted elements that are bigger than the element we're inserting to the right. 
        // (There will always be room to the right because we'll be pulling the element that we're inserting out of the array, that will happen to be just beyond the indexOfLastSortedElement because that's the same as the currentTaskInQueue-1)

        if (indexOfLastSortedElement == -1){
            indexOfLastSortedElement++;
        } else {
            currentElementBeingSwappedLeft = indexOfLastSortedElement+1;
            // while there's an element to the left of the current element, swap it left until it's in the correct place
            // todo: check if left exists function
            while (currentElementBeingSwappedLeft-1 > -1 && tasks.queue[currentElementBeingSwappedLeft].scheduledTime < tasks.queue[currentElementBeingSwappedLeft-1].scheduledTime){
                swap_tasks_in_queue(currentElementBeingSwappedLeft, currentElementBeingSwappedLeft-1);
                // tempCopyOfTask = tasks.queue[currentElementBeingSwappedLeft-1];
                // tasks.queue[currentElementBeingSwappedLeft-1] = tasks.queue[currentElementBeingSwappedLeft];
                // tasks.queue[currentElementBeingSwappedLeft] = tempCopyOfTask;
                currentElementBeingSwappedLeft--;
            }
            indexOfLastSortedElement++;
        }
    }
    // us_stopwatch_end();
}



/*
range: 1 (the space after the empty space when the first element, 0, is removed) to tasks.numberOfTasks-1 (the last existing task); basically, every index that has a "good" task in it
swap(source, destination)
swap(currentIndex,currentIndex-1)
*/

/*
        copyOfCurrentTaskInQueue = tasks.queue[currentTaskInQueue];


        for (currentSortedElement = indexOfLastSortedElement+1; currentSortedElement > -1; currentSortedElement--){
            if(copyOfCurrentTaskInQueue.scheduledTime < tasks.queue[currentSortedElement].scheduledTime){// TODO: Is there any chance of index out of bounds here?
                // move currentSortedElement right 
                tasks.queue[currentSortedElement+1] = tasks.queue[currentSortedElement];
            } else {
                // correct insertion point found - put in that task!
                tasks.queue[currentSortedElement] = copyOfCurrentTaskInQueue;
                indexOfLastSortedElement++;
            }
        }
    }
*/
    // Edge case analysis:
    // If queue.numberOfTasks == 0, then the first for loop won't execute.
    // If queue.numberOfTasks == 1, then the for loops will execute enough to run the if statement once, which will trigger the else and produce the desired result.
    // If queue.numberOfTasks == 2, 

// TODO: Check the edge cases: What if there aren't any sorted elements, like at the very beginning of this process?
// TODO: Check if there's space in the queue before we add a task.
    //us_stopwatch_end();

// insert a task into the queue and then sort it
static void task_insert_and_sort(task_s *newTask)
{
    // make sure the array isn't full
    if(tasks.numberOfTasks == MAX_NUM_OF_TASKS)
    {
        return;
    }

    tasks.queue[tasks.numberOfTasks] = *newTask;
    tasks.numberOfTasks++;

    // sort_task_queue();
    ians_insertion_sort();
}

// remove a specified task from the queue and then sort it
static void task_remove(uint8_t taskID)
{
    // make sure the array isn't empty
    if(tasks.numberOfTasks == 0)
    {
        #if LOG_LEVEL_TASKS >= LOG_EVENTS
        println("\ttask removal failed, queue already empty");
        #endif
        return;
    }

    task_clear(&tasks.queue[taskID]);
    tasks.numberOfTasks--;

    // shuffle the array leftwards
    for(uint8_t i = taskID; i < tasks.numberOfTasks + 1; i++)
    {
        swap_tasks_in_queue(i + 1, i);
    }
}

/* ************************************************************************** */
// task system functions

void tasks_init(void)
{
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
    // check that the queue isn't full
    if(tasks.numberOfTasks == MAX_NUM_OF_TASKS) return -1;

    // create and init a new task object
    task_s newTask;
    task_clear(&newTask);

    // populate the new task
    newTask.name = name;
    newTask.event_callback = callback;
    newTask.scheduledTime = time;
    newTask.repeat = repeat;

    printf("registering task:(name:%s)(ptr:%p)(time:%d)(repeat:%d)\r\n", 
            newTask.name, newTask.event_callback, newTask.scheduledTime, newTask.repeat);

    // add it to the queue
    task_insert_and_sort(&newTask);

    return 0;
}

void task_manager_update(void)
{
    // return early if the queue is empty
    if(tasks.numberOfTasks == 0) return;

    // return early if the first task isn't ready yet
    uint24_t currentTime = systick_read();
    if(tasks.queue[FIRST_TASK].scheduledTime > currentTime) return;
    printf("task is ready @%u ", currentTime);
    // print_task(&tasks.queue[FIRST_TASK]);

    // Make sure we don't execute a null function pointer
    if(tasks.queue[FIRST_TASK].event_callback == NULL){
        println(">>> NULL POINTER EXCEPTION <<< ");
        // while(1); // trap
    }

    // execute it
    printf("executing task:(name:%s)(ptr:%p)(time:%d)(repeat:%d)\r\n", 
            tasks.queue[FIRST_TASK].name, tasks.queue[FIRST_TASK].event_callback, 
            tasks.queue[FIRST_TASK].scheduledTime, tasks.queue[FIRST_TASK].repeat);
    tasks.queue[FIRST_TASK].event_callback();

    // if the task should be repeated, re-register it
    if(tasks.queue[FIRST_TASK].repeat != 0)
    {
        // grab a copy of the current task
        task_s currentTask = tasks.queue[FIRST_TASK];

        currentTask.scheduledTime = currentTime + currentTask.repeat;
        task_insert_and_sort(&currentTask);
    }

    // remove the task from the list
    task_remove(FIRST_TASK);
}

/* ************************************************************************** */

/*  Notes on task debugging

    I was getting really wierd null pointer issues that turned out to be caused
    by XC8 optimizing away the task_xxxx() functions, because it can't tell that
    they've been called.

    One attempt to fix it is to change the task function pointer signature to
    something other that void *ptr(void). This didn't make a difference.

    One attempt is to add fake calls the functions in question. If you place
    direct calls to the functions in question AFTER the last return statement
    in another function, then the compiler will bitch about non-reachable code,
    but doesn't remove the task functions.

    Another attempt involves using an assembly directive to register the
    function as a symbol from the assembler's perspective. This has the
    dramatically undesirable side effect of generating pointless reentrant
    copies of any functions that are called from inside the task functions,
    because the compiler still can't properly identify where they belong on the
    call graph.

    Adding --STACK=hybrid to the CFLAGS in the Makefile makes SOME difference.
    The null pointer error is gone, the task functions seem to be making it
    through the compiler. The indirect function calls are still not being
    completed.

*/
// asm("GLOBAL _task_beep");
void task_beep(void)
{
    uint24_t currentTime = systick_read();
    printf("beep %d\r\n", currentTime);
}

// asm("GLOBAL _task_boop");
void task_boop(void)
{
    uint24_t currentTime = systick_read();
    printf("boop %d\r\n", currentTime);
}

// asm("GLOBAL _task_fizz");
void task_fizz(void)
{
    uint24_t currentTime = systick_read();
    printf("fizz %d\r\n", currentTime);
}

// asm("GLOBAL _task_buzz");
void task_buzz(void)
{
    uint24_t currentTime = systick_read();
    printf("buzz %d\r\n", currentTime);
}

void task_self_test(void)
{
    println("");
    println("");
    println("=====");
    println("Check function pointer values");
    printf("task_beep: %p\r\n", task_beep);
    printf("task_boop: %p\r\n", task_boop);
    printf("task_fizz: %p\r\n", task_fizz);
    printf("task_buzz: %p\r\n", task_buzz);
    delay_ms(10);

    println("");
    println("=====");
    println("Task setup");
    task_register((char*)"fizz", task_fizz, 3000, 0);
    task_register((char*)"buzz", task_buzz, 4000, 0);
    task_register((char*)"boop", task_boop, 2000, 0);
    task_register((char*)"beep", task_beep, 1000, 0);
    delay_ms(10);

    println("");
    println("=====");
    println("Confirm task queue is sorted");
    print_task_queue();
}