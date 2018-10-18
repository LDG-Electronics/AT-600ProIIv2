#ifndef _LOG_H_
#define _LOG_H_

#include <stdint.h>

/* ************************************************************************** */
/*  log_level_t
    This is essentially a key:value object that represents the current logging
    level of each registered file
*/
typedef struct {
    const char *name;
    uint8_t *levelPtr;
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

// shell program to edit registered logging levels
int program_logedit_begin(int argc, char **argv);
int program_logedit_continue(int argc, char **argv);

/* ************************************************************************** */

#endif