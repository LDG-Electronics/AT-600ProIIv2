#ifndef _LOG_H_
#define _LOG_H_

#include <stdint.h>

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

extern void log_level_edit(uint8_t file, uint8_t level);

extern void print_log_list(void);

int program_logedit_begin(int argc, char **argv);

int program_logedit_continue(int argc, char **argv);

/* ************************************************************************** */

#endif