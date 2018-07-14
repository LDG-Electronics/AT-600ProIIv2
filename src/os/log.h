#ifndef _LOG_H_
#define _LOG_H_

/* ************************************************************************** */

// Logging levels
#define L_SILENT 0
#define L_FATAL 1
#define L_ERROR 2
#define L_WARN 3
#define L_INFO 4
#define L_DEBUG 5
#define L_TRACE 6

/* ************************************************************************** */

#define log_trace(ARG)                                                         \
    if (log_header(L_TRACE, LOG_LEVEL, __FILE__, __LINE__) == 0) {             \
        ARG                                                                    \
    }
#define log_debug(ARG)                                                         \
    if (log_header(L_DEBUG, LOG_LEVEL, __FILE__, __LINE__) == 0) {             \
        ARG                                                                    \
    }
#define log_info(ARG)                                                          \
    if (log_header(L_INFO, LOG_LEVEL, __FILE__, __LINE__) == 0) {              \
        ARG                                                                    \
    }
#define log_warn(ARG)                                                          \
    if (log_header(L_WARN, LOG_LEVEL, __FILE__, __LINE__) == 0) {              \
        ARG                                                                    \
    }
#define log_error(ARG)                                                         \
    if (log_header(L_ERROR, LOG_LEVEL, __FILE__, __LINE__) == 0) {             \
        ARG                                                                    \
    }
#define log_fatal(ARG)                                                         \
    if (log_header(L_FATAL, LOG_LEVEL, __FILE__, __LINE__) == 0) {             \
        ARG                                                                    \
    }

/* ************************************************************************** */

int8_t log_header(uint8_t msgLevel, uint8_t localLevel, const char *file,
                  int line);

#endif