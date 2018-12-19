#include "sh_relays.h"
#include "../flags.h"
#include "../os/serial_port.h"
#include "../relays.h"
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

void shell_relays(int argc, char **argv) {
    relays_t relays = read_current_relays();
    switch (argc) {
    case 1:
        println("relays set <caps|inds|z> <value>");
        println("relays setall <caps> <inds> <z>");
        println("relays <cup|cdn|lup|ldn|bypass|max>");
        return;
    case 2:
        if (strcmp(argv[1], "cup") == 0) {
            capacitor_increment();
        } else if (strcmp(argv[1], "cdn") == 0) {
            capacitor_decrement();
        } else if (strcmp(argv[1], "lup") == 0) {
            inductor_increment();
        } else if (strcmp(argv[1], "ldn") == 0) {
            inductor_decrement();
        } else if (strcmp(argv[1], "bypass") == 0) {
            enter_bypass();
        } else if (strcmp(argv[1], "max") == 0) {
            set_relays_to_max();
        } else {
            break;
        }
        print_relays(read_current_relays());
        println("");
        return;
    case 4:
        if (strcmp(argv[1], "set") == 0) {
            if (strcmp(argv[2], "caps") == 0) {
                relays.caps = atoi(argv[3]);
            } else if (strcmp(argv[2], "inds") == 0) {
                relays.inds = atoi(argv[3]);
            } else if (strcmp(argv[2], "z") == 0) {
                relays.z = atoi(argv[3]);
            } else {
                break;
            }
            if (put_relays(relays) == -1) {
                // TODO: what do we do on relayerror?
            }
            print_relays(read_current_relays());
            println("");
            return;
        } else if (strcmp(argv[1], "setall") == 0) {
            relays.caps = atoi(argv[2]);
            relays.inds = atoi(argv[3]);
            relays.z = atoi(argv[4]);
            if (put_relays(relays) == -1) {
                // TODO: what do we do on relayerror?
            }
        } else {
            break;
        }

    default:
        break;
    }

    println("invalid arguments");
    return;
}