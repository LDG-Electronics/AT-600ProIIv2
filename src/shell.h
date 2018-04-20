#ifndef _SHELL_H_
#define _SHELL_H_

/* ************************************************************************** */

typedef union {
    struct {
        unsigned delimiterReceived : 1;
    };
    uint8_t all;
}shell_flags_s;

extern volatile shell_flags_s shell_flags;

/* ************************************************************************** */

extern void shell_init(void);

extern void check_for_shell_command(void);

#endif