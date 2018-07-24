#ifndef _HARDWARE_H_
#define _HARDWARE_H_

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
#define BUTTON_PIN_NAME_FORMAT(NAME) NAME##_BUTTON_PIN

/* ************************************************************************** */

extern void startup(void);
extern void shutdown(void);

#endif /* _HARDWARE_H_ */