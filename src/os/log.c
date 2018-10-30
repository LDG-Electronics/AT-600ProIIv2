#include "log.h"
#include "serial_port.h"
#include "shell/shell.h"
#include "shell/shell_keys.h"
#include "shell/shell_utils.h"
#include "system_time.h"
#include <ctype.h>
#include <stdbool.h>
#include <string.h>

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
        logDatabase.file[i].levelPtr = NULL;
    }
    logDatabase.numberOfFiles = 0;
}

void log_register__(const char *name, uint8_t *levelPtr) {
    // make sure we're not float-registering
    for (uint8_t i = 0; i < MAX_NUMBER_OF_FILES; i++) {
        if (!strcmp(name, logDatabase.file[i].name)) {
            return; // file is already registered
        }
    }

    // We're good, register the file
    logDatabase.file[logDatabase.numberOfFiles].name = name;
    logDatabase.file[logDatabase.numberOfFiles].levelPtr = levelPtr;
    logDatabase.numberOfFiles++;
}

/* ************************************************************************** */

void log_header(uint8_t msgLevel, const char *file, int line) {
    reset_text_attributes();
    printf("%lu ", (uint32_t)systick_read());

    printf("%s%-5s", level_colors[msgLevel], level_names[msgLevel]);

    print("\033[1;37m");
    printf(" %s:%d: ", file, line);

    print("\033[0;37;40m");
}

/* ************************************************************************** */

void log_level_edit(uint8_t fileID, uint8_t level) {
    *logDatabase.file[fileID].levelPtr = level;
}

void print_log_level(uint8_t level) {
    printf("%s%-6s ", level_colors[level], level_names[level]);
    reset_text_attributes();
}

void print_names_of_log_levels(void) {
    for (uint8_t i = 0; i < 7; i++) {
        printf("%s%s ", level_colors[i], level_names[i]);
    }
    reset_text_attributes();
}

void print_managed_log_table(void) {
    for (uint8_t i = 0; i < logDatabase.numberOfFiles; i++) {
        printf(" #%-2d | ", (int)i);
        uint8_t level = *logDatabase.file[i].levelPtr;
        printf("%s%-6s", level_colors[level], level_names[level]);
        print("\033[0;37m | ");
        println(logDatabase.file[i].name);
    }
}

void print_log_list(void) {
    println("-----------------------------------------------");
    println("");
    printf("%d files are currently registered.\r\n", logDatabase.numberOfFiles);
    println("");
    println(" #  | level  | path/to/file");
    println("-----------------------------------------------");
    print_managed_log_table();
    println("-----------------------------------------------");
    print_names_of_log_levels();
    println("");
    reset_text_attributes();
    println("");
    println("These levels can be changed with this command:");
    println("log set <#> <level>");
    println("-----------------------------------------------");
}

/* -------------------------------------------------------------------------- */

uint8_t selectedLine = 0;
uint8_t selectedLevel = 0;

void reprint_line(void) {
    uint8_t level = *logDatabase.file[selectedLine].levelPtr;
    term_cursor_home();
    term_cursor_right(7);
    print_log_level(level);
    term_cursor_home();
    term_cursor_right(7);
}

void highlight_line(void) {
    uint8_t level = *logDatabase.file[selectedLine].levelPtr;
    term_cursor_home();
    term_cursor_right(7);
    print("\033[7m"); // invert colors
    print_log_level(level);
    term_cursor_home();
    term_cursor_right(7);
}

void logedit_keys(key_t key) {
    switch (key.key) {
    default:
        return;
    case UP:
        if (selectedLine > 0) {
            reprint_line();
            selectedLine--;
            selectedLevel = *logDatabase.file[selectedLine].levelPtr;
            term_cursor_up(1);
            highlight_line();
        }
        return;
    case DOWN:
        if (selectedLine < logDatabase.numberOfFiles - 1) {
            reprint_line();
            selectedLine++;
            selectedLevel = *logDatabase.file[selectedLine].levelPtr;
            term_cursor_down(1);
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

int8_t logedit_callback(char currentChar) {
    if (iscntrl(currentChar)) {
        key_t key = identify_key(currentChar);
        logedit_keys(key);
    }

    return 0;
}

void logedit(int argc, char **argv) {
    term_reset_screen();

    println("-----------------------------------------------");
    println("");
    printf("%d files are currently registered.\r\n", logDatabase.numberOfFiles);
    println("");
    println("  #  | level  | path/to/file");
    println("-----------------------------------------------");
    print_managed_log_table();
    println("-----------------------------------------------");
    print_names_of_log_levels();
    println("");
    println("press ctrl+c to exit logedit");
    println("-----------------------------------------------");

    term_cursor_set(0, 0);
    term_cursor_down(6);
    selectedLine = 0;

    highlight_line();

    shell_register_callback(logedit_callback);
}