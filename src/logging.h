#ifndef _LOGGING_H_
#define _LOGGING_H_

// Testing revealed that only RESET and BRIGHT are valid attributes in the
// current Raspberry Pi/ GNU screen debugging environment
enum textAttribute {
    RESET = 0,
    BRIGHT = 1,
    DIM = 2,
    UNDERLINE = 3,
    BLINK = 5,
    REVERSE = 7,
};

// All colors are valid
enum textColor {
    BLACK = 0,
    RED = 1,
    GREEN = 2,
    YELLOW = 3,
    BLUE = 4,
    MAGENTA = 5,
    CYAN = 6,
    WHITE = 7,
};

/* ************************************************************************** */

/*  Notes on using the log system:
    
    This module is intended to strike a balance between 'simple enough to embed'
    and 'user-friendly'.
*/

extern void log_init(void);

// Just prints a CR/LF
extern void log_ln(void);

// Prints a string
extern void log_str(const char *string);
extern void log_str_ln(const char *string);

// Converts an integer to a string and prints it
extern void log_int(int32_t value);

// Prints a string and an integer
extern void log_cat(const char *string, int32_t value);
extern void log_cat_ln(const char *string, int32_t value);

// Prints a string and a double
extern void log_catf(const char *string, double value);
extern void log_catf_ln(const char *string, double value);

// Prints the contents of relay struct as "(caps, inds, z)"
extern void log_relays(relays_s *relays);
extern void log_relays_ln(relays_s *relays);

// Prints the current Forward, Reverse, and SWR
extern void log_current_SWR(void);
extern void log_current_SWR_ln(void);

// Change the format of debug output
extern void log_format(enum textAttribute attribute, enum textColor foreground);
extern void log_format_reset(void);

#endif