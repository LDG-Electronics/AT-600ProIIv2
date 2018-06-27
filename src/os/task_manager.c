#include "../includes.h"

/* ************************************************************************** */
/*  Notes on the event system

    This is a minimalist background event scheduler that's designed to complement
    a superloop instead of replacing it.

    event_scheduler_update() needs to be called regularly in the main loop or
    equivalent structure. Any possible event callback should be relatively
    short, contain no majors delays, and should cause limited side-effects.

    Timing is not particularly strict in this scheduler, especially because it
    allows the main program to block at will. This is desirable behavior in many
    embedded devices.
*/

/* ************************************************************************** */
/*  This structure stores the information related to a single event.

    The fields are:
    name: a string that identifies the event
    eventCallback: a function pointer that event a event performs
    scheduledTime: the event will be performed when the current system time
                    equals this scheduled time
    repeat: if the event is to be repeated, it will be re-registered this number
            of system ticks into the future
*/
typedef struct {
    const char *name;
    event_callback_t eventCallback;
    system_time_t scheduledTime;
    uint16_t repeat;
} event_t;

/* ************************************************************************** */
/*  The maximum number of events appears to be limited by the PIC18's RAM page
    size. A page of RAM is 256 bytes, and a event object is 11 bytes long.
    Therefore, the maximum length of the queue is 11 * 23 = 253

*/
#define EVENT_QUEUE_LENGTH 25
#define FIRST_TASK 0

// This structure stores the event queue and related data.
struct {
    event_t queue[EVENT_QUEUE_LENGTH];
    uint8_t numberOfEvents;
    uint8_t nextEvent;
} events;

/* ************************************************************************** */
// event debug utilities

static void print_event(event_t *event) {
    printf("event:(name:%s)(ptr:%p)(time:%u)(repeat:%u)\r\n", event->name,
           event->eventCallback, event->scheduledTime, event->repeat);
}

static void print_event_queue(void) {
    println("");
    println("-----");
    printf("Event queue contains: %d events\r\n", events.numberOfEvents);
    println("");

    if (events.numberOfEvents == 0)
        return;

    // step through the list and print each event
    for (uint8_t i = 0; i < events.numberOfEvents; i++) {
        print_event(&events.queue[i]);
        // every fifth event, add a line break
        if (((i + 1) % 5) == 0) {
            println("");
        }
    }

    println("-----");
    println("");
}

/* -------------------------------------------------------------------------- */
// Event queue manipulation utilities

#define queue_is_full() (events.numberOfEvents == EVENT_QUEUE_LENGTH)
#define queue_is_empty() (events.numberOfEvents == 0)
#define copy_event_in_queue(source, destination)                                \
    (events.queue[destination] = events.queue[source])
#define event_is_ready(eventIndex, currentTime)                                  \
    (events.queue[eventIndex].scheduledTime < currentTime)
#define get_event_scheduled_time(eventIndex)                                     \
    (events.queue[eventIndex].scheduledTime)

// Delete the contents of a given event, setting its elements to 'clean' values
static void event_clear(event_t *event) {
    event->name = NULL;
    event->eventCallback = NULL;
    event->scheduledTime = UINT24_MAX;
    event->repeat = 0;
}

// swap two events
static void swap_events_in_queue(uint8_t eventA, uint8_t eventB) {
    event_t copyOfEventB = events.queue[eventB];
    events.queue[eventB] = events.queue[eventA];
    events.queue[eventA] = copyOfEventB;
}

/* -------------------------------------------------------------------------- */

static void event_queue_sort(void) {
    uint8_t currentEventInQueue;

    int8_t indexOfLastSortedElement = -1;
    uint8_t currentElementBeingSwappedLeft;

    // If there aren't any events, then what are we doing here?
    if (queue_is_empty()) {
#if LOG_LEVEL_TASKS >= LOG_EVENTS
        println("Event Queue is empty!");
        println("Can't sort an empty list!");
#endif

        return;
    }

    // The queue can't be out of order if there's only one event
    if (events.numberOfEvents == 1) {
#if LOG_LEVEL_TASKS >= LOG_EVENTS
        println("Event Queue only contains one event!");
        println("Can't sort a single item!");
#endif

        return;
    }

    for (currentEventInQueue = 0; currentEventInQueue < events.numberOfEvents;
         currentEventInQueue++) {
        if (indexOfLastSortedElement == -1) {
            indexOfLastSortedElement++;
        } else {
            currentElementBeingSwappedLeft = indexOfLastSortedElement + 1;
            // while there's an element to the left of the current element, swap
            // it left until it's in the correct place
            while (currentElementBeingSwappedLeft - 1 > -1 &&
                   events.queue[currentElementBeingSwappedLeft].scheduledTime <
                       events.queue[currentElementBeingSwappedLeft - 1]
                           .scheduledTime) {
                swap_events_in_queue(currentElementBeingSwappedLeft,
                                    currentElementBeingSwappedLeft - 1);
                currentElementBeingSwappedLeft--;
            }
            indexOfLastSortedElement++;
        }
    }
}

// insert a event into the queue and then sort it
static void insert_event_and_sort(event_t *newEvent) {
    if (queue_is_full()) {
#if LOG_LEVEL_TASKS >= LOG_EVENTS
        println("event insertion failed, queue is full");
#endif
        return;
    }

    events.queue[events.numberOfEvents] = *newEvent;
    events.numberOfEvents++;

    event_queue_sort();
}

// remove a specified event from the queue and then sort what's left
static void remove_event(uint8_t eventIndex) {
    // make sure the array isn't empty
    if (queue_is_empty()) {
#if LOG_LEVEL_TASKS >= LOG_EVENTS
        println("event removal failed, queue already empty");
#endif
        return;
    }

    // Remove the event
    event_clear(&events.queue[eventIndex]);
    events.numberOfEvents--;

    // TODO: prove that this is shuffling properly
    // shuffle the array leftwards
    for (uint8_t i = eventIndex; i < events.numberOfEvents + 1; i++) {
        copy_event_in_queue(i + 1, i);
    }
}

/* ************************************************************************** */
// event system functions

void event_scheduler_init(void) {
    // initialize the queue
    events.numberOfEvents = 0;
    events.nextEvent = 0;

    for (uint8_t i = 0; i < EVENT_QUEUE_LENGTH; i++) {
        event_clear(&events.queue[i]);
    }
}

/* -------------------------------------------------------------------------- */

/*  event_scheduler_update() should be called from application code at regular
    intervals, during periods where no time-critical work is being done.

    The project this event manager was designed for has a series of nested state
    machine loops that are used to poll user input and respond.
*/
void event_scheduler_update(void) {
    if (queue_is_empty())
        return;

    // return early if the first event isn't ready yet
    system_time_t currentTime = systick_read();
    if (!event_is_ready(FIRST_TASK, currentTime))
        return;

#if LOG_LEVEL_TASKS >= LOG_EVENTS
    printf("event is ready @%u\r\n", currentTime);
    print("executing");
    print_event(&events.queue[FIRST_TASK]);
#endif

    // Make sure we don't execute a null function pointer
    if (events.queue[FIRST_TASK].eventCallback != NULL) {
        // execute it
        events.queue[FIRST_TASK].eventCallback();

        // if the event should be repeated, re-register it
        if (events.queue[FIRST_TASK].repeat != 0) {
            // grab a copy of the current event
            event_t repeatingEvent = events.queue[FIRST_TASK];
            // update its scheduledTime
            repeatingEvent.scheduledTime = currentTime + repeatingEvent.repeat;
            // throw it back in the queue
            insert_event_and_sort(&repeatingEvent);
        }
    } else { // pointer is null, do not execute
#if LOG_LEVEL_TASKS >= LOG_ERROR
        println(">>> NULL POINTER EXCEPTION <<< ");
#endif
        // while(1); // trap
    }

    remove_event(FIRST_TASK);
}

/* -------------------------------------------------------------------------- */

// create a event object and add it to the event queue
int8_t event_register(const char *name, event_callback_t callback,
                     system_time_t time, uint16_t repeat) {
    if (queue_is_full())
        return -1;

    // create and init a new event object
    event_t newEvent;
    event_clear(&newEvent);

    // populate the new event
    newEvent.name = name;
    newEvent.eventCallback = callback;
    newEvent.scheduledTime = time + systick_read();
    newEvent.repeat = repeat;

#if LOG_LEVEL_TASKS >= LOG_EVENTS
    print("Registering ");
    print_event(&events.queue[FIRST_TASK]);
#endif

    // add it to the queue
    insert_event_and_sort(&newEvent);

    return 0;
}

/* -------------------------------------------------------------------------- */

// remove a event from the queue using its name as a key
int8_t event_deregister(const char *name) {
    int8_t index = event_queue_lookup(name);

    // the indicated event was not found
    if (index == -1)
        return -1;

    // remove the event
    remove_event(index);

    return 0;
}

// check if a event with the given name exists in the queue
int8_t event_queue_lookup(const char *name) {
    for (uint8_t i = 0; i < EVENT_QUEUE_LENGTH; i++) {
        if (!strcmp(name, events.queue[i].name)) {
            return 0; // found a match
        }
    }
    return -1; // didn't find a match
}

/* ************************************************************************** */
/*  Event Manager testing utilities

*/
// dummy events to be used as callbacks in event_telf_test()
void event_beep(void) { printf("beep %d\r\n", systick_read()); }
void event_boop(void) { printf("boop %d\r\n", systick_read()); }
void event_fizz(void) { printf("fizz %d\r\n", systick_read()); }
void event_buzz(void) { printf("buzz %d\r\n", systick_read()); }
void dummy_event(void) { printf("time: %d\r\n", systick_read()); }

/* ************************************************************************** */

// print out the size in bytes of the various objects used in the queue
void print_object_sizes(void) {
    println("");
    println("=====");
    println("Check size of event queue objects");
    printf("sizeof events: %d bytes\r\n", sizeof(events));
    printf("sizeof events: %d bytes\r\n", sizeof(events.queue));
    printf("sizeof events: %d bytes\r\n", sizeof(events.queue[0]));
}

// print the values of the function pointers to cross-reference against other
// output
void print_pointer_values(void) {
    println("");
    println("=====");
    println("Check function pointer values");
    printf("event_beep: %p\r\n", event_beep);
    printf("event_boop: %p\r\n", event_boop);
    printf("event_fizz: %p\r\n", event_fizz);
    printf("event_buzz: %p\r\n", event_buzz);
}

const system_time_t random_times[] = {
    6750, 5698, 8424, 3556, 1678, 6438, 8957, 586,  600,  531,  // 10 items
    4511, 106,  3014, 1309, 3065, 4848, 8870, 244,  3183, 3777, // 20 items
    2675, 6938, 6995, 1841, 1171, 6554, 7143, 5970, 7052, 1622, // 30 items
    // 7810, 2342, 7783, 4372, 8031, 9432, 6134, 3114, 3247, 4294, // 40 items
    // 2768, 138, 3279, 4376, 7459, 3443, 1670, 7893, 3908, 1025, // 50 items
    // 8667, 9322, 8222, 9365, 8866, 9266, 3125, 6630, 6584, 6697, // 60 items
    // 1683, 8651, 9668, 9218, 9391, 299, 906, 8828, 2042, 609 // 70 items
};

void event_queue_fill_test(void) {
    println("");
    println("=====");
    println("Event setup");

    uint8_t i = 0;

    // fill up that queue
    event_register("1", dummy_event, random_times[i++], 0);
    event_register("2", dummy_event, random_times[i++], 0);
    event_register("3", dummy_event, random_times[i++], 0);
    event_register("4", dummy_event, random_times[i++], 0);
    event_register("5", dummy_event, random_times[i++], 0);

    event_register("6", dummy_event, random_times[i++], 0);
    event_register("7", dummy_event, random_times[i++], 0);
    event_register("8", dummy_event, random_times[i++], 0);
    event_register("9", dummy_event, random_times[i++], 0);
    event_register("10", dummy_event, random_times[i++], 0);

    event_register("11", dummy_event, random_times[i++], 0);
    event_register("12", dummy_event, random_times[i++], 0);
    event_register("13", dummy_event, random_times[i++], 0);
    event_register("14", dummy_event, random_times[i++], 0);
    event_register("15", dummy_event, random_times[i++], 0);

    event_register("16", dummy_event, random_times[i++], 0);
    event_register("17", dummy_event, random_times[i++], 0);
    event_register("18", dummy_event, random_times[i++], 0);
    event_register("19", dummy_event, random_times[i++], 0);
    event_register("20", dummy_event, random_times[i++], 0);

    event_register("21", dummy_event, random_times[i++], 0);
    event_register("22", dummy_event, random_times[i++], 0);
    event_register("23", dummy_event, random_times[i++], 0);
    event_register("24", dummy_event, random_times[i++], 0);
    event_register("25", dummy_event, random_times[i++], 0);

    // event_register("26", dummy_event, random_times[i++], 0);
    // event_register("27", dummy_event, random_times[i++], 0);
    // event_register("28", dummy_event, random_times[i++], 0);
    // event_register("29", dummy_event, random_times[i++], 0);
    // event_register("30", dummy_event, random_times[i++], 0);

    // event_register("31", dummy_event, random_times[i++], 0);
    // event_register("32", dummy_event, random_times[i++], 0);
    // event_register("33", dummy_event, random_times[i++], 0);
    // event_register("34", dummy_event, random_times[i++], 0);
    // event_register("35", dummy_event, random_times[i++], 0);

    println("");
    println("=====");
    println("Confirm event queue is sorted");
    print_event_queue();
}

void event_torting_simple_test(void) {
    println("");
    println("=====");
    println("Event setup");
    event_register("fizz", event_fizz, 3000, 0);
    event_register("buzz", event_buzz, 4000, 0);
    event_register("boop", event_boop, 2000, 0);
    event_register("beep", event_beep, 1000, 0);

    println("");
    println("=====");
    println("Confirm event queue is sorted");
    print_event_queue();
}

void event_torting_stress_test(void) {}

void event_manager_self_test(void) {
    println("");

    // print_object_sizes();

    // print_pointer_values();

    event_queue_fill_test();

    // event_torting_simple_test();

    // event_torting_stress_test();
}