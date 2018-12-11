#include "../events.h"
#include "../os/serial_port.h"
#include <string.h>

/* ************************************************************************** */

void tune(int argc, char **argv) {
    switch (argc) {
    case 1:
        request_full_tune();
        return;
    case 2:
        if (!strcmp(argv[1], "full")) {
            request_full_tune();
            return;
        }
        if (!strcmp(argv[1], "mem")) {
            request_memory_tune();
            return;
        }
        break;
    default:
        break;
    }
    println("invalid arguments");
    return;
}