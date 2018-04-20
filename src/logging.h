#ifndef _LOGGING_H_
#define _LOGGING_H_

/* ************************************************************************** */

/*  Notes on using the log system:
    
    This module is intended to strike a balance between 'simple enough to embed'
    and 'user-friendly'.
*/

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

#endif