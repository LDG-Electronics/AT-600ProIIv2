#ifndef _SHELL_SEQUENCES_H_
#define _SHELL_SEQUENCES_H_

/* ************************************************************************** */

#define KEY_NAME_LIST                                                          \
    X(UNKNOWN)                                                                 \
    X(HOME)                                                                    \
    X(END)                                                                     \
    X(INSERT)                                                                  \
    X(DELETE)                                                                  \
    X(PAGEUP)                                                                  \
    X(PAGEDOWN)                                                                \
    X(UP)                                                                      \
    X(DOWN)                                                                    \
    X(LEFT)                                                                    \
    X(RIGHT)                                                                   \
    X(F1)                                                                      \
    X(F2)                                                                      \
    X(F3)                                                                      \
    X(F4)                                                                      \
    X(F5)                                                                      \
    X(F6)                                                                      \
    X(F7)                                                                      \
    X(F8)                                                                      \
    X(F9)                                                                      \
    X(F10)                                                                     \
    X(F11)                                                                     \
    X(F12)

#define KEY_MODIFIER_LIST                                                      \
    X(NONE)                                                                    \
    X(BLANK)                                                                   \
    X(SHIFT)                                                                   \
    X(ALT)                                                                     \
    X(SHIFT_ALT)                                                               \
    X(CTRL)                                                                    \
    X(SHIFT_CTRL)                                                              \
    X(CTRL_ALT)                                                                \
    X(CTRL_ALT_SHIFT)

#define X(NAME) NAME,
typedef enum { KEY_NAME_LIST } key_names_t;
typedef enum { KEY_MODIFIER_LIST } key_modifiers_t;
#undef X

extern const char *keyNameString[];
extern const char *keyModifierString[];

typedef struct {
    key_names_t key;
    key_modifiers_t mod;
} key_t;

/* ************************************************************************** */

// setup
extern void shell_sequences_init(void);

// returns a key object that matches 
extern key_t intercept_escape_sequence(void);

#endif