#include "../includes.h"
#include <string.h>
static uint8_t LOG_LEVEL = L_SILENT;

/* ************************************************************************** */
/*  Notes on the event system

    This is a minimalist background event scheduler that's designed to
    complement a superloop instead of replacing it.

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
    registrationTime: used to calculate elapsed time
    executionTime: event is ready when elapsedTime > executionTime
    repeat: if the event is to be repeated, it will be re-registered this number
            of system ticks into the future
*/
typedef struct {
    const char *name;
    event_callback_t eventCallback;
    system_time_t registrationTime;
    system_time_t executionTime;
    unsigned exists : 1;
} event_t;

// Delete the contents of a given event, setting its elements to 'clean' values
static void event_clear(event_t *event) {
    event->name = NULL;
    event->eventCallback = NULL;
    event->registrationTime = UINT24_MAX;
    event->executionTime = UINT24_MAX;
    event->exists = 0;
}

/* ************************************************************************** */
/*  The maximum number of events appears to be limited by the PIC18's RAM page
    size. A page of RAM is 256 bytes.

    sizeof(events) * EVENT_QUEUE_LENGTH must be less than 256

*/
#define EVENT_QUEUE_LENGTH 16

// This structure stores the event queue and related data.
struct {
    event_t queue[EVENT_QUEUE_LENGTH];
    uint8_t numberOfEvents;
    uint8_t nextEvent;
} events;

/* ************************************************************************** */

void event_scheduler_init(void) {
    // initialize the queue
    events.numberOfEvents = 0;
    events.nextEvent = 0;

    for (uint8_t i = 0; i < EVENT_QUEUE_LENGTH; i++) {
        event_clear(&events.queue[i]);
    }

    log_register();
}

/* ************************************************************************** */
// event debug utilities

// TODO: printf and 24 bit ints are not cooperating
// try casting to uint24_t?

static void print_event(event_t *event) {
    printf("event:(name:%s)(ptr:%p)(reg:%d)(exec:%d)\r\n", event->name,
           event->eventCallback, event->registrationTime, event->executionTime);
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
            delay_ms(50);
        }
    }

    println("-----");
    println("");
}

/* -------------------------------------------------------------------------- */
// Event queue manipulation utilities

// queue length checking
#define queue_is_full() (events.numberOfEvents == EVENT_QUEUE_LENGTH)
#define queue_is_empty() (events.numberOfEvents == 0)

// return when the event was registered
#define time_since_registration(eventIndex)                                    \
    systick_elapsed_time(events.queue[(eventIndex)].registrationTime)

// return how much time is left until an event is ready
#define time_until_ready(eventIndex)                                           \
    time_since_registration(eventIndex) - events.queue[eventIndex].executionTime

// return 1 if the event is ready, 0 if it is not
static int8_t event_is_ready(eventIndex) {
    if (time_since_registration(eventIndex) >=
        events.queue[eventIndex].executionTime)
        return 1;
    return 0;
}

/* -------------------------------------------------------------------------- */

// find out which event will be ready next
static void recalculate_next_event(void) {
    LOG_TRACE({ println("recalculate_next_event"); });
    uint8_t validEventsChecked = 0;
    uint8_t bestEventIndex = events.nextEvent;
    system_time_t bestEventTime = time_until_ready(bestEventIndex);

    // check the whole event queue and find the soonest event
    for (uint8_t i = 0; i < EVENT_QUEUE_LENGTH; i++) {
        if (time_until_ready(i) > bestEventTime) {
            bestEventIndex = i;
            bestEventTime = time_until_ready(i);
        }
        if (events.queue[i].exists)
            validEventsChecked++;
        if (validEventsChecked >= events.numberOfEvents)
            break;
    }

    LOG_DEBUG({ printf("nextEvent: %d\r\n", events.nextEvent); });
    // update the next event
    events.nextEvent = bestEventIndex;
}

// insert a event into the queue
static void insert_event(event_t *newEvent) {
    LOG_TRACE({ println("insert_event"); });
    if (queue_is_full()) {
        LOG_INFO({ println("event insertion failed, queue is full"); });
        return;
    }

    // TODO: this does not handle a fragmented queue properly
    events.queue[events.numberOfEvents] = *newEvent;
    events.numberOfEvents++;

    LOG_DEBUG({ printf("queue now contains: %d\r\n", events.numberOfEvents); });
    recalculate_next_event();
}

// remove a specified event from the queue
static void remove_event(uint8_t eventIndex) {
    LOG_TRACE({ println("remove_event"); });
    // make sure the array isn't empty
    if (queue_is_empty()) {
        LOG_INFO({ println("event removal failed, queue already empty"); });
        return;
    }

    // Remove the event
    event_clear(&events.queue[eventIndex]);
    events.numberOfEvents--;

    LOG_DEBUG({ printf("queue now contains: %d\r\n", events.numberOfEvents); });
    recalculate_next_event();
}

/* ************************************************************************** */
// event system functions

/*  event_scheduler_update() should be called from application code at regular
    intervals, during periods where no time-critical work is being done.

    The project this event manager was designed for has a series of nested state
    machine loops that are used to poll user input and respond.
*/
system_time_t previousTime = 0;

void event_scheduler_update(void) {
    // return early if there's nothing to do
    if (queue_is_empty()) {
        return;
    }

    // return early if the first event isn't ready yet
    if (!event_is_ready(events.nextEvent)) {
        return;
    }

    // grab the time, in case we need to re-register the event
    system_time_t currentTime = systick_read();

    LOG_INFO({
        printf("event is ready, last event was %lu ago\r\n",
               (uint32_t)systick_elapsed_time(previousTime));
        previousTime = currentTime;
    });

    LOG_INFO({
        print("executing ");
        print_event(&events.queue[events.nextEvent]);
    });

    // Make sure we don't execute a null function pointer
    if (events.queue[events.nextEvent].eventCallback == NULL) {
        LOG_FATAL({ println(">>> NULL POINTER EXCEPTION <<< "); });
        // while(1); // trap
        return;
    }

    // Execute the event
    int16_t repeat = events.queue[events.nextEvent].eventCallback();

    // if the event should be repeated, re-register it
    if (repeat > 0) {
        LOG_INFO({
            print("Reregistering: ");
            print_event(&events.queue[events.nextEvent]);
        });
        events.queue[events.nextEvent].registrationTime = currentTime;
        events.queue[events.nextEvent].executionTime = repeat;

        recalculate_next_event();
    } else if (repeat == 0) {
        remove_event(events.nextEvent);
    } else if (repeat < 0) {
        remove_event(events.nextEvent);
        // TODO: I need an adult
    }
}

/* -------------------------------------------------------------------------- */

// create a event object and add it to the event queue
int8_t event_register(const char *name, event_callback_t callback,
                      system_time_t time) {
    LOG_TRACE({ println("event_register"); });
    if (queue_is_full()) {
        LOG_ERROR(println("event queue is full!"););
        return -1;
    }

    // create and init a new event object
    event_t newEvent;
    event_clear(&newEvent);

    // populate the new event
    newEvent.name = name;
    newEvent.eventCallback = callback;
    newEvent.registrationTime = systick_read();
    // TODO: is there a time calculation bug here?
    newEvent.executionTime = time;
    newEvent.exists = 1;

    LOG_INFO({
        print("Registering: ");
        print_event(&newEvent);
    });

    // add it to the queue
    insert_event(&newEvent);

    return 0;
}

/* -------------------------------------------------------------------------- */

// remove a event from the queue using its name as a key
int8_t event_deregister(const char *name) {
    LOG_TRACE({ println("event_deregister"); });
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
    LOG_TRACE({ println("event_queue_lookup"); });
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
int16_t event_beep(void) {
    printf("beep %d\r\n", (uint32_t)systick_read());
    return 0;
}
int16_t event_boop(void) {
    printf("boop %d\r\n", (uint32_t)systick_read());
    return 0;
}
int16_t event_fizz(void) {
    printf("fizz %d\r\n", (uint32_t)systick_read());
    return 0;
}
int16_t event_buzz(void) {
    printf("buzz %d\r\n", (uint32_t)systick_read());
    return 0;
}
int16_t dummy_event(void) {
    printf("time: %d\r\n", (uint32_t)systick_read());
    return 0;
}

/* ************************************************************************** */

// print out the size in bytes of the various objects used in the queue
static void print_object_sizes(void) {
    println("");

    println("=====");
    println("Check size of event queue objects");
    printf("sizeof events: %d bytes\r\n", sizeof(events));
    printf("sizeof events.queue: %d bytes\r\n", sizeof(events.queue));
    printf("sizeof events.queue[0]: %d bytes\r\n", sizeof(events.queue[0]));
}

// print the values of the function pointers to cross-reference against other
// output
static void print_pointer_values(void) {
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

static void event_queue_fill_test(void) {
    println("");
    println("=====");
    println("Event setup");

    uint8_t i = 0;

    // fill up that queue
    event_register("1", dummy_event, random_times[i++]);
    event_register("2", dummy_event, random_times[i++]);
    event_register("3", dummy_event, random_times[i++]);
    event_register("4", dummy_event, random_times[i++]);
    event_register("5", dummy_event, random_times[i++]);

    event_register("6", dummy_event, random_times[i++]);
    event_register("7", dummy_event, random_times[i++]);
    event_register("8", dummy_event, random_times[i++]);
    event_register("9", dummy_event, random_times[i++]);
    event_register("10", dummy_event, random_times[i++]);

    event_register("11", dummy_event, random_times[i++]);
    event_register("12", dummy_event, random_times[i++]);
    event_register("13", dummy_event, random_times[i++]);
    event_register("14", dummy_event, random_times[i++]);
    event_register("15", dummy_event, random_times[i++]);

    event_register("16", dummy_event, random_times[i++]);
    event_register("17", dummy_event, random_times[i++]);
    event_register("18", dummy_event, random_times[i++]);
    event_register("19", dummy_event, random_times[i++]);
    event_register("20", dummy_event, random_times[i++]);

    // event_register("21", dummy_event, random_times[i++]);
    // event_register("22", dummy_event, random_times[i++]);
    // event_register("23", dummy_event, random_times[i++]);
    // event_register("24", dummy_event, random_times[i++]);
    // event_register("25", dummy_event, random_times[i++]);

    // event_register("26", dummy_event, random_times[i++]);
    // event_register("27", dummy_event, random_times[i++]);
    // event_register("28", dummy_event, random_times[i++]);
    // event_register("29", dummy_event, random_times[i++]);
    // event_register("30", dummy_event, random_times[i++]);

    // event_register("31", dummy_event, random_times[i++]);
    // event_register("32", dummy_event, random_times[i++]);
    // event_register("33", dummy_event, random_times[i++]);
    // event_register("34", dummy_event, random_times[i++]);
    // event_register("35", dummy_event, random_times[i++]);

    println("");
    println("=====");
    println("Confirm event queue is sorted");
    print_event_queue();
}

static void event_sorting_simple_test(void) {
    println("");
    println("=====");
    println("Event setup");
    event_register("fizz", event_fizz, 3000);
    event_register("buzz", event_buzz, 4000);
    event_register("boop", event_boop, 2000);
    event_register("beep", event_beep, 1000);

    println("");
    println("=====");
    println("Confirm event queue is sorted");
    print_event_queue();
}

static void event_sorting_stress_test(void) {}

void event_manager_self_test(void) {
    println("");

    print_object_sizes();

    print_pointer_values();

    event_queue_fill_test();

    event_sorting_simple_test();

    event_sorting_stress_test();
}