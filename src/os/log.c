#include "../includes.h"
#include "shell/shell_sequences.h"

/* ************************************************************************** */
const char *level_names[] = {
    "SILENT", "FATAL", "ERROR", "WARN", "INFO", "DEBUG", "TRACE",
};
const char *level_colors[] = {
    "\033[1;94m", "\033[1;35m", "\033[1;31m", "\033[1;33m",
    "\033[1;32m", "\033[1;36m", "\033[1;34m",
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

void log_register__(const char *name, uint8_t *level) {
    logDatabase.file[logDatabase.numberOfFiles].name = name;
    logDatabase.file[logDatabase.numberOfFiles].level = level;

    logDatabase.numberOfFiles++;
}

void log_level_edit(uint8_t fileID, uint8_t level) {
    *logDatabase.file[fileID].level = level;
}

void print_log_list(void) {
    println("-----------------------------------------------");
    for (uint8_t i = 0; i < 7; i++) {
        printf("%s%s ", level_colors[i], level_names[i]);
    }
    println("");
    println("");

    print("\033[0;37;40m");

    printf("%d files are currently registered.\r\n", logDatabase.numberOfFiles);
    for (uint8_t i = 0; i < logDatabase.numberOfFiles; i++) {
        printf("#%d : ", (int)i);
        uint8_t level = *logDatabase.file[i].level;
        printf("%s%-6s", level_colors[level], level_names[level]);
        print("\033[0;37m : ");
        println(logDatabase.file[i].name);
    }

    println("-----------------------------------------------");
}

int program_log_edit(int argc, char **argv) {
    char currentchar;

    println("press ctrl+c to exit logedit");
    println("");

    print_log_list();

    while (1) {
        currentchar = getch();
        if (!currentchar) {
            continue;
        }

        // ctrl + c, exit program
        if (currentchar == 3) {
            return;
        }

        if (currentchar == 27) {
            key_t key = intercept_escape_sequence();
        }


        
    }
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

#if 0
void log_message_test(void) {
    uint8_t test = 1;

    log_trace(printf("trace log entry #%d\r\n", test++););
    log_debug(printf("debug log entry #%d\r\n", test++););
    log_info(printf("info log entry #%d\r\n", test++););
    log_warn(printf("warn log entry #%d\r\n", test++););
    log_error(printf("error log entry #%d\r\n", test++););
    log_fatal(printf("fatal log entry #%d\r\n", test++););
}
#endif