#include "../includes.h"

#include "shell/shell_command_processing.h"
#include "shell/shell_keys.h"
#include <ctype.h>

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

    shell_register_command(program_logedit_begin, "logedit", NULL);
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
    println("");
    printf("%d files are currently registered.\r\n", logDatabase.numberOfFiles);
    println("");
    println(" #  | level  | path/to/file");
    println("-----------------------------------------------");
    for (uint8_t i = 0; i < logDatabase.numberOfFiles; i++) {
        printf(" #%-1d | ", (int)i);
        uint8_t level = *logDatabase.file[i].level;
        printf("%s%-6s", level_colors[level], level_names[level]);
        print("\033[0;37m | ");
        println(logDatabase.file[i].name);
    }

    println("-----------------------------------------------");
    for (uint8_t i = 0; i < 7; i++) {
        printf("%s%s ", level_colors[i], level_names[i]);
    }
    println("\033[0;37;40m");
    println("log set <#> <level>");
    println("-----------------------------------------------");
}

/* -------------------------------------------------------------------------- */
// "reverse" text decoration
// print("\033[7m");

uint8_t selectedLine = 0;
uint8_t selectedLevel = 0;

void reprint_line(void) {
    uint8_t level = *logDatabase.file[selectedLine].level;
    print("\033[100D"); // move cursor to left edge
    print("\033[0m");   // reset text attributes
    print("\033[6C");   // move cursor right to correct position
    printf("%s%-6s", level_colors[level], level_names[level]);
}

void highlight_line(void) {
    uint8_t level = *logDatabase.file[selectedLine].level;
    print("\033[100D"); // move cursor to left edge
    print("\033[0m");   // reset text attributes
    print("\033[6C");   // move cursor right to correct position
    print("\033[7m");   // invert colors
    printf("%s%-6s", level_colors[level], level_names[level]);
    print("\033[0m"); // reset text attributes
}

void logedit_keys(key_t key) {
    switch (key.key) {
    case UP:
        if (selectedLine > 0) {
            reprint_line();
            selectedLine--;
            selectedLevel = *logDatabase.file[selectedLine].level;
            print("\033[1A");
            highlight_line();
        }
        return;
    case DOWN:
        if (selectedLine < logDatabase.numberOfFiles - 1) {
            reprint_line();
            selectedLine++;
            selectedLevel = *logDatabase.file[selectedLine].level;
            print("\033[1B");
            highlight_line();
        }
        return;
    case LEFT:
        if (selectedLevel > 0) {
            selectedLevel--;
            log_level_edit(selectedLine, selectedLevel);
            highlight_line();
        }
        return;
    case RIGHT:
        if (selectedLevel < 6) {
            selectedLevel++;
            log_level_edit(selectedLine, selectedLevel);
            highlight_line();
        }
        return;
    }
}

int program_logedit(int argc, char **argv) {
    char currentChar = *argv[0];
    if (iscntrl(currentChar)) {
        key_t key = identify_key(currentChar);
        logedit_keys(key);
    }
    return 0;
}

int program_logedit_begin(int argc, char **argv) {
    // clear screen and reset cursor to (0,0)
    print("\033[2J");
    println("-----------------------------------------------");
    println("");
    printf("%d files are currently registered.\r\n", logDatabase.numberOfFiles);
    println("");
    println(" #  | level  | path/to/file");
    println("-----------------------------------------------");

    for (uint8_t i = 0; i < logDatabase.numberOfFiles; i++) {
        printf(" #%-1d | ", (int)i);
        uint8_t level = *logDatabase.file[i].level;
        printf("%s%-6s", level_colors[level], level_names[level]);
        print("\033[0;37m | ");
        println(logDatabase.file[i].name);
    }

    println("-----------------------------------------------");
    for (uint8_t i = 0; i < 7; i++) {
        printf("%s%s ", level_colors[i], level_names[i]);
    }
    print("\033[0m"); // reset text attributes
    println("");
    println("press ctrl+c to exit logedit");
    println("-----------------------------------------------");

    // reset cursor to (0,0)
    print("\033[0;0H");
    print("\033[6B");
    selectedLine = 0;

    highlight_line();

    shell_set_program_callback(program_logedit);
    return 1;
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