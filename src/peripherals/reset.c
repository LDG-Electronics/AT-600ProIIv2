#include "../includes.h"

#include "../os/log_macros.h"
#define LOG_LEVEL L_FATAL

/* ************************************************************************** */

void reset_vector_handler(void) {
    if (PCON0bits.STKOVF == 1) {
        LOG_FATAL({ println("\r\n\r\n>>> Stack Overflow <<<"); });
        while (1) {
            // trap
        }
    }

    if (PCON0bits.STKUNF == 1) {
        LOG_FATAL({ println("\r\n\r\n>>> Stack Underflow <<<"); });
        while (1) {
            // trap
        }
    }

    if (PCON0bits.RMCLR == 0) {
        LOG_FATAL(println("\r\n\r\n>>> MCLR RESET <<<"););
        while (1) {
            // trap
        }
    }

    if (PCON0bits.RI == 0) {
        LOG_FATAL(println("\r\n\r\n>>> RESET <<<"););
        while (1) {
            // trap
        }
    }
}