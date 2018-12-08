#include "../display.h"
#include "../os/serial_port.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

/* ************************************************************************** */

void shell_show_bargraphs(int argc, char **argv) {
    switch (argc) {
    case 1:
        println("usage: bar <forward> <SWR>");
        return;
    case 3:
        print("first arg: ");
        print(argv[1]);
        float forwardWatts = atof(argv[1]);
        printf(", forwardWatts: %f\r\n", forwardWatts);

        print("second arg: ");
        print(argv[2]);
        float swrValue = atof(argv[2]);
        printf(", swrValue: %f\r\n", swrValue);

        display_frame_t frame = render_RF(forwardWatts, swrValue);

        println("");
        println("Rendered frame:");
        print_frame(frame);

        displayBuffer.next = frame;
        display_update();
        return;
    default:
        break;
    }

    println("invalid arguments");
    return;
}
