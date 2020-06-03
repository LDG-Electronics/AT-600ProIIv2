#ifndef _HARDWARE_H_
#define _HARDWARE_H_

/* ************************************************************************** */
// version information

/* [[[cog cog.outl(f'#define PRODUCT_NAME "{utils.project.name}"') ]]] */
#define PRODUCT_NAME "AT-600ProII"
/* [[[end]]] */

/* ************************************************************************** */
/* [[[cog
    timers = {f'timer{k}': [] for k in range(7)}
    for f in utils.all_files('src', '*.c'):
        text = open(f).read()
        for t in timers:
            if t in text:
                timers[t].append(f[4:])

    cog.outl(utils.start_comment + '  //' + '! Timer registry')
    for t in timers:
        cog.out(f'    {t} is ')
        if timers[t]:
            cog.out('used in: ' + ', '.join(timers[t]))
        else:
            cog.out('unused' )
        cog.outl('')
    cog.outl(utils.end_comment)
]]] */
/*  //! Timer registry
    timer0 is used in: os/stopwatch.c
    timer1 is unused
    timer2 is used in: os/system_time.c
    timer3 is used in: rf_sensor.c
    timer4 is used in: rf_sensor.c
    timer5 is unused
    timer6 is used in: os/buttons.c
*/
/* [[[end]]] */

/* ************************************************************************** */
// Button stuff

// AT-600ProII has 8 front panel buttons
#define NUMBER_OF_BUTTONS 8

// This is an X Macro that is used to populate the button driver
#define BUTTON_LIST                                                            \
    X(TUNE)                                                                    \
    X(FUNC)                                                                    \
    X(CUP)                                                                     \
    X(CDN)                                                                     \
    X(LUP)                                                                     \
    X(LDN)                                                                     \
    X(ANT)                                                                     \
    X(POWER)

// What is the format of the button pin names?
/*  Buttons are wired with pullups to 5v, and are active-low. This helps prevent
    random extra button presses caused by the noisy, highRF environment.

    Consequently, inputs need to be inverted.
*/
#define BUTTON_PIN_NAME_FORMAT(NAME) read_##NAME##_BUTTON_PIN

/* ************************************************************************** */

#endif // _HARDWARE_H_