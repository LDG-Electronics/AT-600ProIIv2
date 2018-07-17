#ifndef _LOG_H_
#define _LOG_H_

/* ************************************************************************** */

extern const char *level_names[];

extern const char *level_colors[];

typedef enum {
    L_SILENT,
    L_FATAL,
    L_ERROR,
    L_WARN,
    L_INFO,
    L_DEBUG,
    L_TRACE,
} log_levels_t;

/* ************************************************************************** */

typedef struct {
    const char *name;
    uint8_t *level;
} log_level_t;

#define MAX_NUMBER_OF_FILES 20

typedef struct {
    log_level_t file[MAX_NUMBER_OF_FILES];
    uint8_t numberOfFiles;
} log_database_t;

extern log_database_t logDatabase;

/* ************************************************************************** */

// setup
extern void log_init(void);

extern void log_register__(const char *name, uint8_t *level);
#define log_register() log_register__(__FILE__, &logLevel)

extern void log_level_edit(uint8_t file, uint8_t level);

extern void print_log_list(void);

int program_logedit_begin(int argc, char **argv);

int program_logedit_continue(int argc, char **argv);

/* ************************************************************************** */

// TODO: implement ian's suggestions RE: LOG_LEVEL and logLevel
// TODO: add braces to log statements
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

#endif