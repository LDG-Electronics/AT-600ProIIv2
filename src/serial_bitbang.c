#include "includes.h"

/* ************************************************************************** */

/*  serial_init: initialize timer6 and any other relevant config regs
    for _bitbang_ serial transmission. Timer 6 is used for timing operations.
    For 9600 baud, with a 32 MHz system clock, the cloc period is as follows:

    9600 baud = 104.166 uS per bit

    32 MHz / 4 (fosc/4) = 8 MHz
    8 MHz / 16 (1:16 prescaler) = 500000 Hz
    500000 Hz = 2 uS / tick
    52 ticks * 2 uS = 104 uS, or 0.15% error 
 */

void serial_bitbang_init(void)
{
    BITBANG_PIN = 1; // Put bitbang serial line at idle state
    
    timer6_init();
    timer6_start();
}

/*  serial_tx_char() is configured for serial at 115200 baud with one stop bit.
    
    Bitbang serial driver uses timer6 to space bits. Timer should be left running
    to ensure consistent spacing of bytes and eliminate jitter.
*/
void serial_tx_char(const char c)
{
    uint8_t i;
    char a;

    di();

    BITBANG_PIN = 1;

    while (TIMER6_IF == 0);
    TIMER6_IF = 0;

    while (TIMER6_IF == 0);
    TIMER6_IF = 0;


    BITBANG_PIN = 0;

    for (i = 0; i <= 7; i++)
    {
        a = ((c >> i) & 0x01);
        while (TIMER6_IF == 0);
        TIMER6_IF = 0;

        BITBANG_PIN = a;
    }

    while (TIMER6_IF == 0);
    TIMER6_IF = 0;

    BITBANG_PIN = 1;

    ei();
}

// Bitbang a null-terminated string of up to 255 characters
void serial_tx_string(const char *string)
{
    uint8_t i;
    i = 0;
    while (string[i])
    {
        serial_tx_char(string[i++]);
    }
}
