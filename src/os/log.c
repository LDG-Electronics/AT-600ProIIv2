#include "../includes.h"

/* ************************************************************************** */

static const char *level_names[] = {
    "SILENT", "FATAL", "ERROR", "WARN", "INFO", "DEBUG", "TRACE",
};

static const char *level_colors[] = {
    "\033[1;94m", "\033[1;35m", "\033[1;31m", "\033[1;33m",
    "\033[1;32m", "\033[1;36m", "\033[1;94m",
};

log_database_t logDatabase;

/* -------------------------------------------------------------------------- */

void log_init(void) {
    for (uint8_t i = 0; i < MAX_NUMBER_OF_FILES; i++) {
        logDatabase.file[i].name = NULL;
        logDatabase.file[i].level = NULL;
    }
    logDatabase.numberOfFiles = 0;
}

void log_register(const char *name, uint8_t *level) {
    logDatabase.file[logDatabase.numberOfFiles].name = name;
    logDatabase.file[logDatabase.numberOfFiles].level = level;

    logDatabase.numberOfFiles++;
}

void print_log_list(void) {
    println("-----------------------------------------------");
    printf("%d files are currently registered.\r\n", logDatabase.numberOfFiles);
    for (uint8_t i = 0; i < logDatabase.numberOfFiles; i++) {
        uint8_t level = *logDatabase.file[i].level;
        printf("%s%-6s", level_colors[level], level_names[level]);
        print("\033[0;37m : ");
        println(logDatabase.file[i].name);
    }
    println("-----------------------------------------------");
}

/* ************************************************************************** */

int8_t log_header(uint8_t msgLevel, uint8_t localLevel, const char *file,
                  int line) {

    if (msgLevel > localLevel) {
        return -1;
    }

    print("\033[0;37m");
    printf("%lu ", (uint32_t)systick_read());

    printf("%s%-5s", level_colors[msgLevel], level_names[msgLevel]);

    print("\033[1;37m");
    printf(" %s:%d: ", file, line);

    print("\033[0;37;40m");

    return 0;
}

void log_message_test(void) {
    uint8_t test = 1;

    log_trace(printf("trace log entry #%d\r\n", test++););
    log_debug(printf("debug log entry #%d\r\n", test++););
    log_info(printf("info log entry #%d\r\n", test++););
    log_warn(printf("warn log entry #%d\r\n", test++););
    log_error(printf("error log entry #%d\r\n", test++););
    log_fatal(printf("fatal log entry #%d\r\n", test++););
}