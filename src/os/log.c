#include "../includes.h"

/* ************************************************************************** */

static const char *level_names[] = {
    "SILENT", "FATAL", "ERROR", "WARN", "INFO", "DEBUG", "TRACE",
};

static const char *level_colors[] = {
    "SILENT",     "\033[1;35m", "\033[1;31m", "\033[1;33m",
    "\033[1;32m", "\033[1;36m", "\033[1;94m",
};

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