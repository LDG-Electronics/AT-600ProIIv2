#ifndef _LOG_MACROS_H_
#define _LOG_MACROS_H_

#include <stdbool.h>
#include <stdint.h>

#include "console_io.h"
#include "log.h"

/* ************************************************************************** */

bool log_header(uint8_t msgLevel, uint8_t localLevel, const char *file,
                int line);

#define LOG_TRACE(ARG)                                                         \
    if (log_header(L_TRACE, LOG_LEVEL, __FILE__, __LINE__)) {                  \
        ARG                                                                    \
    }
#define LOG_DEBUG(ARG)                                                         \
    if (log_header(L_DEBUG, LOG_LEVEL, __FILE__, __LINE__)) {                  \
        ARG                                                                    \
    }
#define LOG_INFO(ARG)                                                          \
    if (log_header(L_INFO, LOG_LEVEL, __FILE__, __LINE__)) {                   \
        ARG                                                                    \
    }
#define LOG_WARN(ARG)                                                          \
    if (log_header(L_WARN, LOG_LEVEL, __FILE__, __LINE__)) {                   \
        ARG                                                                    \
    }
#define LOG_ERROR(ARG)                                                         \
    if (log_header(L_ERROR, LOG_LEVEL, __FILE__, __LINE__)) {                  \
        ARG                                                                    \
    }
#define LOG_FATAL(ARG)                                                         \
    if (log_header(L_FATAL, LOG_LEVEL, __FILE__, __LINE__)) {                  \
        ARG                                                                    \
    }

/* ************************************************************************** */

#endif