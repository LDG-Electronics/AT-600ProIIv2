#include "includes.h"

/* ************************************************************************** */

void shell_commands_init(void) {

    // contents
    shell_register(shell_get_RF, "getRF");
}

/* -------------------------------------------------------------------------- */
// Shell command for RF sensor
int shell_get_RF(int argc, char** argv)
{
    if(argc == 1) {
        print_current_SWR_ln();
    } else {
        if(!strcmp(argv[1], "-fwd")) {
            printf("%d\r\n", currentRF.forward);
        } else if(!strcmp(argv[1], "-rev")) {
            printf("%d\r\n", currentRF.reverse);
        } else if(!strcmp(argv[1], "-swr")) {
            printf("%f\r\n", currentRF.swr);
        } else if(!strcmp(argv[1], "-freq")) {
            printf("%d\r\n", currentRF.frequency);
        } else {
            println("invalid argument");
        }
    }   

    return SHELL_RET_SUCCESS;
}