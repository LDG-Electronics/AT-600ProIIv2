#include "flags.h"
#include "os/serial_port.h"
#include "os/shell/shell_command_processor.h"
#include "relays.h"
#include <stdlib.h>
#include <string.h>

/* ************************************************************************** */

void capacitor_increment(void) {
    relays_t relays = read_current_relays();

    if (relays.caps < MAX_CAPACITORS) {
        relays.caps++;
        if (put_relays(relays) == -1) {
            relays.caps--;
        }
    }
}

void capacitor_decrement(void) {
    relays_t relays = read_current_relays();

    if (relays.caps > MIN_CAPACITORS) {
        relays.caps--;
        if (put_relays(relays) == -1) {
            relays.caps++;
        }
    }
}

void inductor_increment(void) {
    relays_t relays = read_current_relays();

    if (relays.inds < MAX_INDUCTORS) {
        relays.inds++;
        if (put_relays(relays) == -1) {
            relays.inds--;
        }
    }
}

void inductor_decrement(void) {
    relays_t relays = read_current_relays();

    if (relays.inds > MIN_INDUCTORS) {
        relays.inds--;
        if (put_relays(relays) == -1) {
            relays.inds++;
        }
    }
}

void enter_bypass(void) {
    relays_t relays = read_current_relays();

    relays.all = 0;
    if (put_relays(relays) == -1) {
        // TODO: what do we do on relayerror?
    }
}

void set_relays_to_max(void) {
    relays_t relays = read_current_relays();

    relays.inds = MAX_INDUCTORS;
    relays.caps = MAX_CAPACITORS;
    if (put_relays(relays) == -1) {
        // TODO: what do we do on relayerror?
    }
}

void sh_relays(int argc, char **argv) {
    relays_t relays = read_current_relays();
    switch (argc) {
    case 1: // usage
        println("relays set <caps|inds|z|ant> <value>");
        println("relays setall <caps> <inds> <z>");
        println("relays <cup|cdn|lup|ldn|bypass|max>");
        return;
    case 2: // relays <cup|cdn|lup|ldn|bypass|max>
        if (!strcmp(argv[1], "cup")) {
            capacitor_increment();
        } else if (!strcmp(argv[1], "cdn")) {
            capacitor_decrement();
        } else if (!strcmp(argv[1], "lup")) {
            inductor_increment();
        } else if (!strcmp(argv[1], "ldn")) {
            inductor_decrement();
        } else if (!strcmp(argv[1], "bypass")) {
            enter_bypass();
        } else if (!strcmp(argv[1], "max")) {
            set_relays_to_max();
        } else {
            break;
        }

        print_relays(read_current_relays());
        println("");
        return;
    case 4: // relays set <caps|inds|z|ant> <value>
        if (!strcmp(argv[1], "set")) {
            // decode relays
            if ((!strcmp(argv[2], "caps")) || (!strcmp(argv[2], "c"))) {
                relays.caps = atoi(argv[3]);
            } else if ((!strcmp(argv[2], "inds")) || (!strcmp(argv[2], "l"))) {
                relays.inds = atoi(argv[3]);
            } else if (!strcmp(argv[2], "z")) {
                relays.z = atoi(argv[3]);
            } else if ((!strcmp(argv[2], "ant")) || (!strcmp(argv[2], "a"))) {
                systemFlags.antenna = atoi(argv[3]);
                relays = read_current_relays();
            } else {
                break;
            }

            // publish
            if (put_relays(relays) == -1) {
                // TODO: what do we do on relayerror?
            }

            print_relays(read_current_relays());
            println("");
            return;
        }
        break;
    case 5: // relays setall <caps> <inds> <z>
        if (!strcmp(argv[1], "setall")) {
            // decode relays
            relays.caps = atoi(argv[2]);
            relays.inds = atoi(argv[3]);
            relays.z = atoi(argv[4]);

            // publish
            if (put_relays(relays) == -1) {
                // TODO: what do we do on relayerror?
            }

            print_relays(read_current_relays());
            println("");
            return;
        }
        break;

    default:
        break;
    }

    println("invalid arguments");
    return;
}