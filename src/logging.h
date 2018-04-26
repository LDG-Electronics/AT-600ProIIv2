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

#endif