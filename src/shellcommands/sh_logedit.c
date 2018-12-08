#include "sh_logedit.h"
#include "../os/log.h"
#include "../os/serial_port.h"
#include "../os/shell/shell.h"
#include "../os/shell/shell_keys.h"
#include "../os/shell/shell_utils.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

/* ************************************************************************** */

uint8_t selectedLine = 0;
uint8_t selectedLevel = 0;

void reprint_line(void) {
    term_cursor_home();
    term_cursor_right(7);
    print_log_level(*logDatabase.file[selectedLine].levelPtr);
    term_cursor_home();
    term_cursor_right(7);
}

/* -------------------------------------------------------------------------- */

void print_logedit_header(void) {
    println("");
    printf("%d files are currently registered.\r\n", logDatabase.numberOfFiles);
    println("");
    println("  #  | level  | path/to/file");
}

void print_managed_log_table(void) {
    for (uint8_t i = 0; i < logDatabase.numberOfFiles; i++) {
        printf(" #%-2d", (int)i);
        print(" | ");
        print_log_level(*logDatabase.file[i].levelPtr);
        print(" | ");
        println(logDatabase.file[i].name);
    }
}

void print_names_of_log_levels(void) {
    for (uint8_t i = 0; i < 7; i++) {
        print_log_level(i);
        print(" ");
    }
    println("");
}

void print_logedit_footer(void) {
    println("press f5 to refresh list");
    println("press ctrl+c to exit logedit");
}

void draw_logedit(void) {
    // wipe the screen
    term_reset_screen();

    // draw charts and graphs
    println("-----------------------------------------------");
    print_logedit_header();
    println("-----------------------------------------------");
    print_managed_log_table();
    println("-----------------------------------------------");
    print_names_of_log_levels();
    println("-----------------------------------------------");
    print_logedit_footer();
    println("-----------------------------------------------");

    // set the cursor to the first log level
    term_cursor_set(0, 0);
    term_cursor_down(6);
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
    selectedLevel = *logDatabase.file[selectedLine].levelPtr;
    term_cursor_up(1);
    invert_text_attribute();
    reprint_line();
}

void down_one_line(void) {
    reprint_line();
    selectedLine++;
    selectedLevel = *logDatabase.file[selectedLine].levelPtr;
    term_cursor_down(1);
    invert_text_attribute();
    reprint_line();
}

void increase_level(void) {
    selectedLevel++;
    log_level_edit(selectedLine, selectedLevel);
    invert_text_attribute();
    reprint_line();
}

void decrease_level(void) {
    selectedLevel--;
    log_level_edit(selectedLine, selectedLevel);
    invert_text_attribute();
    reprint_line();
}

void logedit_keys(key_t key) {
    switch (key.key) {
    default:
        return;
    case UP:
        if (selectedLine > 0) {
            up_one_line();
        }
        return;
    case DOWN:
        if (selectedLine < logDatabase.numberOfFiles - 1) {
            down_one_line();
        }
        return;
    case LEFT:
        if (selectedLevel > 0) {
            decrease_level();
        }
        return;
    case RIGHT:
        if (selectedLevel < 6) {
            increase_level();
        }
        return;
    case F5:
        draw_logedit();
    }
}

/* ************************************************************************** */

int8_t logedit_callback(char currentChar) {
    if (iscntrl(currentChar)) {
        key_t key = identify_key(currentChar);
        logedit_keys(key);
    }

    return 0;
}

void logedit(int argc, char **argv) {
    draw_logedit();

    shell_register_callback(logedit_callback);
}