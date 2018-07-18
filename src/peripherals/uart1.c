#include "../includes.h"

#include "../libraries/fast_ring_buffer.h"
#include "uart1.h"

/* ************************************************************************** */

volatile uart_buffer_s UART1_tx_buffer;
volatile uart_buffer_s UART1_rx_buffer;

void UART1_baud_select(UART_baud_rates baudRate) {
    U1BRG = baudTable[baudRate];
}

void UART1_init(UART_baud_rates baudRate) {
    UART1_baud_select(baudRate);

    U1CON0bits.BRGS = 1; // Baud Rate is set to high speed
    U1CON0bits.TXEN = 1; // Transmit is enabled
    U1CON0bits.RXEN = 1; // Recieve is enabled

    // initialize ring buffer pointers
    UART1_tx_buffer.head = 0;
    UART1_tx_buffer.tail = 0;
    UART1_rx_buffer.head = 0;
    UART1_rx_buffer.tail = 0;

    PIE3bits.U1RXIE = 1; // Enable UART1 Recieve Interrupt

    U1CON1bits.ON = 1; // Enable UART1
}

/* -------------------------------------------------------------------------- */
// UART1 transmit

/*  Notes on UART1_tx_ISR()

    This function is an Interrupt Vector Table compatible ISR to respond to the
    IRQ_U1TX interrupt signal. This signal is generated whenever U1TXB is empty
    and PIE3bits.U1TXIE is enabled.

*/

#define UART1_TX_IE_enable() PIE3bits.U1TXIE = 1
#define UART1_TX_IE_disable() PIE3bits.U1TXIE = 0

void __interrupt(irq(IRQ_U1TX), high_priority) UART1_tx_ISR() {
    if (buffer_is_empty(UART1_tx_buffer)) {
        UART1_TX_IE_disable();
    } else {
        U1TXB = buffer_read(UART1_tx_buffer);
    }
}

/*  Notes on using UART1_tx_string()

    The following block handles the case when this function overflows the
    UART1_tx_buffer. There are three options for dealing with this input
    overflow:

    1:  Check available space before writing to buffer, and abort and return an
        error if the string won't fit. This doesn't fit with the desired API.
    2:  Silently overwrite the contents of the buffer. This is obviously
        unsatisfactory.
    3:  Block until there's enough room. This is the winner.

    If the buffer is ever full, block until there's enough room to fit the
    remainder of the input string. If there's more string left than BUFFER_SIZE,
    just wait until the buffer is totally empty, then continue filling the
    buffer. This will have an undesirable ping-pong effect until the string is
    over, but that's better than breaking our API or clobbering existing data.
*/
void UART1_tx_string(const char *string, const char terminator) {
    uint16_t currentByte = 0;

    // loop until hitting null
    while (string[currentByte] != terminator) {
        // buffer overflow handler
        if (buffer_is_full(UART1_tx_buffer)) {
            // If the buffer is ever full, wait for it to empty completely
            UART1_TX_IE_enable();
            while (!buffer_is_empty(UART1_tx_buffer))
                ;
        }

        begin_critical_section();
        buffer_write(UART1_tx_buffer, string[currentByte++]);
        end_critical_section();
    }

    UART1_TX_IE_enable();
}

void UART1_tx_char(char data) {
    // buffer overflow handler
    if (buffer_is_full(UART1_tx_buffer)) {
        UART1_TX_IE_enable();

        while (buffer_is_full(UART1_tx_buffer))
            ;
    }

    begin_critical_section();
    buffer_write(UART1_tx_buffer, data);
    end_critical_section();

    UART1_TX_IE_enable();
}

/* -------------------------------------------------------------------------- */
// UART1 receive

/*  Notes on UART1_rx_ISR()

    This function is an Interrupt Vector Table compatible ISR to respond to the
    IRQ_U1RX interrupt signal. This signal is generated whenever there is an
    unread byte in U1RXB.
*/

void __interrupt(irq(IRQ_U1RX), high_priority) UART1_rx_ISR() {
    buffer_write(UART1_rx_buffer, U1RXB);
}

char UART1_rx_char(void) {
    char data = 0;

    if (buffer_is_empty(UART1_rx_buffer))
        return '\0';

    begin_critical_section();
    data = buffer_read(UART1_rx_buffer);
    end_critical_section();

    return data;
}