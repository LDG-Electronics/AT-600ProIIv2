#include "sh_logedit.h"
#include "../os/log.h"
#include "../os/log_macros.h"
#include "../os/serial_port.h"
#include "../os/shell/shell.h"
#include "../os/shell/shell_keys.h"
#include "../os/shell/shell_utils.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

/* ************************************************************************** */

uint8_t oldLogDatabase[MAX_NUMBER_OF_FILES];
uint8_t newLogDatabase[MAX_NUMBER_OF_FILES];

void set_global_to_silent(void) {
    for (uint8_t i = 0; i < MAX_NUMBER_OF_FILES; i++) {
        *logDatabase.file[i].levelPtr = L_SILENT;
    }
}

void global_to_local(void) {
    for (uint8_t i = 0; i < MAX_NUMBER_OF_FILES; i++) {
        oldLogDatabase[i] = *logDatabase.file[i].levelPtr;
    }
}

void old_to_new(void) {
    for (uint8_t i = 0; i < MAX_NUMBER_OF_FILES; i++) {
        oldLogDatabase[i] = newLogDatabase[i];
    }
}

void old_to_global(void) {
    for (uint8_t i = 0; i < MAX_NUMBER_OF_FILES; i++) {
        *logDatabase.file[i].levelPtr = oldLogDatabase[i];
    }
}

void new_to_global(void) {
    for (uint8_t i = 0; i < MAX_NUMBER_OF_FILES; i++) {
        *logDatabase.file[i].levelPtr = newLogDatabase[i];
    }
}

/* -------------------------------------------------------------------------- */

uint8_t selectedLine = 0;
uint8_t selectedLevel = 0;

#define COLUMN_NUMBER 7

void reprint_line(void) {
    term_cursor_home();
    term_cursor_right(COLUMN_NUMBER);
    print_log_level(newLogDatabase[selectedLine]);
    term_cursor_home();
    term_cursor_right(COLUMN_NUMBER);
}

/* -------------------------------------------------------------------------- */

#define HEADER_HEIGHT 10

void print_logedit_header(void) {
    println("-----------------------------------------------");
    // usage instructions
    println("press f5 to refresh list");
    println("press ENTER to save and exit");
    println("press ESC to exit without saving");
    println("press ctrl+c to terminate logedit");

    println("");
    printf("%d files are currently registered.\r\n", logDatabase.numberOfFiles);
    println("");
    println("  #  | level  | path/to/file");
    println("-----------------------------------------------");
}

void print_managed_log_table(void) {
    for (uint8_t i = 0; i < logDatabase.numberOfFiles; i++) {
        printf(" #%-2d", (int)i);
        print(" | ");
        print_log_level(newLogDatabase[i]);
        print(" | ");
        println(logDatabase.file[i].name);
    }
}

void print_logedit_footer(void) {
    println("-----------------------------------------------");
    for (uint8_t i = 0; i < NUMBER_OF_LOG_LEVELS; i++) {
        print_log_level(i);
        print(" ");
    }
    println("");
    println("-----------------------------------------------");
}

void draw_logedit(void) {
    // wipe the screen
    term_reset_screen();

    // draw charts and graphs
    print_logedit_header();
    print_managed_log_table();
    print_logedit_footer();

    // set the cursor to the first log level
    term_cursor_set(0, 0);
    term_cursor_down(HEADER_HEIGHT);
    selectedLine = 0;
    selectedLevel = 0;

    // highlight the first line
    invert_text_attribute();
    reprint_line();
}

/* -------------------------------------------------------------------------- */

void up_one_line(void) {
    reprint_line();
    selectedLine--;
    selectedLevel = newLogDatabase[selectedLine];
    term_cursor_up(1);
    invert_text_attribute();
    reprint_line();
}

void down_one_line(void) {
    reprint_line();
    selectedLine++;
    selectedLevel = newLogDatabase[selectedLine];
    term_cursor_down(1);
    invert_text_attribute();
    reprint_line();
}

void increase_level(void) {
    selectedLevel++;
    newLogDatabase[selectedLine] = selectedLevel;
    invert_text_attribute();
    reprint_line();
}

void decrease_level(void) {
    selectedLevel--;
    newLogDatabase[selectedLine] = selectedLevel;
    invert_text_attribute();
    reprint_line();
}

int8_t logedit_keys(key_t key) {
    switch (key.key) {
    default:
        return 0;
    case UP:
        if (selectedLine > 0) {
            up_one_line();
        }
        return 0;
    case DOWN:
        if (selectedLine < logDatabase.numberOfFiles - 1) {
            down_one_line();
        }
        return 0;
    case LEFT:
        if (selectedLevel > 0) {
            decrease_level();
        }
        return 0;
    case RIGHT:
        if (selectedLevel < 6) { // todo: magic number
            increase_level();
        }
        return 0;
    case F5:
        draw_logedit();
        return 0;
    case ENTER:
        new_to_global();
        return -1;
    case ESCAPE:
        old_to_global();
        return -1;
    }
}

/* ************************************************************************** */

// loop - executed on every key press
// must have a return to indicate desire to terminate program
// if the correct state to do that comes about during the last keypress
int8_t logedit_callback(char currentChar) {
    if (iscntrl(currentChar)) {
        key_t key = identify_key(currentChar);
        return logedit_keys(key);
    }

    return 0;
}

// setup
void logedit(int argc, char **argv) {
    global_to_local();
    old_to_new();
    set_global_to_silent();

    term_hide_cursor();
    draw_logedit();

    shell_register_callback(logedit_callback);
}