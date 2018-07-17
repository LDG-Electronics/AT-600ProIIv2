#include "../includes.h"
#include "shell/shell_command_processing.h"
#include "shell/shell_keys.h"
static uint8_t LOG_LEVEL = L_DEBUG;

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

    log_register();

    shell_register_command(program_logedit_begin, "logedit", NULL);
}

void log_register__(const char *name, uint8_t *level) {
    logDatabase.file[logDatabase.numberOfFiles].name = name;
    logDatabase.file[logDatabase.numberOfFiles].level = level;

    logDatabase.numberOfFiles++;
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

    print("-----------------------------------------------");
}

int program_logedit_begin(int argc, char **argv) {
    println("press ctrl+c to exit logedit");
    println("");

    print_log_list();
}

int program_logedit_continue(int argc, char **argv) {
    char currentChar;

    // ctrl + c, exit program
    if (currentChar == 3) {
        return;
    }

    if (currentChar == 27) {
        key_t key = identify_key(currentChar);

        LOG_DEBUG({ print_key(&key); });
    }
}

/* ************************************************************************** */

bool log_header(uint8_t msgLevel, uint8_t localLevel, const char *file,
                int line) {
    if (msgLevel > localLevel) {
        return false;
    }

    print("\033[0;37m");
    printf("%lu ", (uint32_t)systick_read());

    printf("%s%-5s", level_colors[msgLevel], level_names[msgLevel]);

    print("\033[1;37m");
    printf(" %s:%d: ", file, line);

    print("\033[0;37;40m");

    return true;
}

#if 0
void log_message_test(void) {
    uint8_t test = 1;

    LOG_TRACE({ printf("trace log entry #%d\r\n", test++); });
    LOG_DEBUG({ printf("debug log entry #%d\r\n", test++); });
    LOG_INFO({ printf("info log entry #%d\r\n", test++); });
    LOG_WARN({ printf("warn log entry #%d\r\n", test++); });
    LOG_ERROR({ printf("error log entry #%d\r\n", test++); });
    LOG_FATAL({ printf("fatal log entry #%d\r\n", test++); });
}
#endif