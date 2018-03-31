#include "includes.h"

/* ************************************************************************** */

void interrupt_init(void)
{
    INTCON0bits.IPEN = 0; // Disable priority levels on interrupts
    
    // Clear all peripheral interrupts
    PIE1 = 0x0;
    PIE2 = 0x0;
    PIE3 = 0x0;
    PIE4 = 0x0;
    PIE5 = 0x0;
    PIE6 = 0x0;
    PIE7 = 0x0;
    PIE8 = 0x0;
    PIE9 = 0x0;
    
    // Clear all peripheral flags
    PIR1 = 0x0;
    PIR2 = 0x0;
    PIR3 = 0x0;
    PIR4 = 0x0;
    PIR5 = 0x0;
    PIR6 = 0x0;
    PIR7 = 0x0;
    PIR8 = 0x0;
    PIR9 = 0x0;
    
    INTCON0bits.GIE = 1;
}

/* -------------------------------------------------------------------------- */

void __interrupt(irq(TMR0), high_priority) TMR1_ISR(void)
{
    timer0_stop();
    timer0_IF_clear();

    stopwatchCount += 0xffff;
    timer0_start();
}

void __interrupt(irq(TMR5), high_priority) TMR5_ISR(void)
{
    timer5_stop();
    timer5_IF_clear();
    TMR5H = 0x63;   // reset timer
    TMR5L = 0xC0;

    // Grab current state of every button
    buttons[TUNE] <<= 1;
    buttons[TUNE] |= TUNE_BUTTON;
    
    buttons[FUNC] <<= 1;
    buttons[FUNC] |= FUNC_BUTTON;
    
    buttons[CUP] <<= 1;
    buttons[CUP] |= CUP_BUTTON;
    
    buttons[CDN] <<= 1;
    buttons[CDN] |= CDN_BUTTON;
    
    buttons[LUP] <<= 1;
    buttons[LUP] |= LUP_BUTTON;
    
    buttons[LDN] <<= 1;
    buttons[LDN] |= LDN_BUTTON;
    
    // buttons[ANT] <<= 1; //! ANT button is disabled
    // buttons[ANT] |= ANT_BUTTON;

    buttons[POWER] <<= 1;
    buttons[POWER] |= POWER_BUTTON;
    
    timer5_start();
}

void __interrupt(irq(IRQ_IOC), high_priority) IOC_ISR()
{   
    // interrupt on change for pin IOCAF3
    if(IOCAFbits.IOCAF3 == 1)
    {
        IOCAFbits.IOCAF3 = 0;
        PIE0bits.IOCIE = 0;
        IOCANbits.IOCAN3 = 0;
    }
}
