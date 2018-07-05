#include "../includes.h"

/* ************************************************************************** */

void reset_vector_handler(void)
{
    if(PCON0bits.STKOVF == 1) {
        println("");
        println("");
        println(">>> Stack Overflow <<<");
        while(1); // trap
    }
    
    if(PCON0bits.STKUNF == 1) {
        println("");
        println("");
        println(">>> Stack Underflow <<<");
        while(1); // trap
    }
    
    // if(PCON0bits.RMCLR == 0) {
    //     println("");
    //     println("");
    //     println(">>> MCLR RESET <<<");
    //     while(1); // trap
    // }
    
    // if(PCON0bits.RI == 0) {
    //     println("");
    //     println("");
    //     println(">>> RESET <<<");
    //     while(1); // trap
    // }
    
}