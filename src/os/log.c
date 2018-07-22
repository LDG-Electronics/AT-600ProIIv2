#include "log.h"
#include "console_io.h"
#include "shell/shell_command_processor.h"
#include "shell/shell_keys.h"
#include "shell/shell_utils.h"
#include "system_time.h"
#include <ctype.h>
#include <stdbool.h>

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
        uint8_t level = *logDatabase.file[i].level;
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
    uint8_t level = *logDatabase.file[selectedLine].level;
    term_cursor_home();
    term_cursor_right(7);
    print_log_level(level);
    term_cursor_home();
    term_cursor_right(7);
}

void highlight_line(void) {
    uint8_t level = *logDatabase.file[selectedLine].level;
    term_cursor_home();
    term_cursor_right(7);
    print("\033[7m"); // invert colors
    print_log_level(level);
    term_cursor_home();
    term_cursor_right(7);
}

void logedit_keys(key_t key) {
    switch (key.key) {
    case UP:
        if (selectedLine > 0) {
            reprint_line();
            selectedLine--;
            selectedLevel = *logDatabase.file[selectedLine].level;
            term_cursor_up(1);
            highlight_line();
        }
        return;
    case DOWN:
        if (selectedLine < logDatabase.numberOfFiles - 1) {
            reprint_line();
            selectedLine++;
            selectedLevel = *logDatabase.file[selectedLine].level;
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

int program_logedit(int argc, char **argv) {
    char currentChar = *argv[0];
    if (iscntrl(currentChar)) {
        key_t key = identify_key(currentChar);
        logedit_keys(key);
    }
    return 0;
}

int program_logedit_begin(int argc, char **argv) {
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

    shell_set_program_callback(program_logedit);
    return 1;
}

/* ************************************************************************** */

bool log_header(uint8_t msgLevel, uint8_t localLevel, const char *file,
                int line) {
    if (msgLevel > localLevel) {
        return false;
    }

    reset_text_attributes();
    printf("%ul ", (uint32_t)systick_read());

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