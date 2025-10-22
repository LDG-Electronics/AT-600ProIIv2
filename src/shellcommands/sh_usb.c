#include "os/serial_port.h"
#include "os/shell/shell_command_processor.h"
#include "os/usb_port.h"
#include <stdlib.h>
#include <string.h>


/* ************************************************************************** */

void sh_usb(int argc, char **argv) {
    println("usb");
    usb_println("usb");
    switch (argc) {
    case 1:
        return;
    case 2:
        
        break;
    case 3:
        
        break;
    default:
        break;
    }
    // println("invalid arguments");
    return;
}