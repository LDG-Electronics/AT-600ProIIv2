#include "includes.h"

/* ************************************************************************** */

// Global RF Readings
RF_power_s currentRF;

/* ************************************************************************** */

void RF_sensor_init(void)
{    
    timer1_init();
    
    currentRF.forward = 0;
    currentRF.reverse = 0;
    currentRF.swr = 0;
    currentRF.period = 0xffff;
}

/* -------------------------------------------------------------------------- */

uint16_t get_freq(void)
{
    timer1_clear();

    TIMER3_IF = 0;
    TMR3H = 0xE0;
    TMR3L = 0xC0;

    timer3_start();
    TIMER1_ON = 1;

    while ((TIMER3_IF) == 0);
    // delay_ms(1);
    
    TIMER1_ON = 0;
    timer3_stop();
    

    return timer1_read();
}