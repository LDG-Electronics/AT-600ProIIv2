#include "uart2.h"
#include "../libraries/fast_ring_buffer.h"
#include "../os/system_time.h"
#include "interrupt.h"
#include "pic18f46k42.h"

/* ************************************************************************** */

volatile fast_ring_buffer_t UART2_tx_buffer;
volatile fast_ring_buffer_t UART2_rx_buffer;

void UART2_baud_select(UART_baud_rates baudRate) {
    U2BRG = baudTable[baudRate];
}

void UART2_init(UART_baud_rates baudRate) {
    UART2_baud_select(baudRate);

    U2CON0bits.BRGS = 1; // Baud Rate is set to high speed
    U2CON0bits.TXEN = 1; // Transmit is enabled
    U2CON0bits.RXEN = 1; // Recieve is enabled

    // initialize ring buffer pointers
    UART2_tx_buffer.head = 0;
    UART2_tx_buffer.tail = 0;
    UART2_rx_buffer.head = 0;
    UART2_rx_buffer.tail = 0;

    PIE6bits.U2RXIE = 1; // Enable UART2 Recieve Interrupt

    U2CON1bits.ON = 1; // Enable UART1
}

/* -------------------------------------------------------------------------- */
// UART2 transmit

/*  Notes on UART2_tx_ISR()

    This function is an Interrupt Vector Table compatible ISR to respond to the
    U2TX interrupt signal. This signal is generated whenever U2TXB is empty and
    PIE3bits.U2TXIE is enabled. In effect, the interrupt flag is set every time
    UART2 finishes transmitting a byte.
*/

#define UART2_TX_IE_enable() PIE6bits.U2TXIE = 1
#define UART2_TX_IE_disable() PIE6bits.U2TXIE = 0

void __interrupt(irq(U2TX), high_priority) UART2_tx_ISR() {
    if (buffer_is_empty(UART2_tx_buffer)) {
        UART2_TX_IE_disable();
    } else {
        U2TXB = buffer_read(UART2_tx_buffer);
    }
}

/*  Notes on using UART2_tx_string()

    Large or frequent calls to UART2_tx_string() can cause the UART2_tx_buffer
    to overflow. There are three options for dealing with this input overflow:

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
void UART2_tx_string(const char *string, const char terminator) {
    uint16_t currentByte = 0;

    // loop until hitting the provided termination character
    while (string[currentByte] != terminator) {
        if (buffer_is_full(UART2_tx_buffer)) {
            UART2_TX_IE_enable();
            delay_ms(10);
        }

        begin_critical_section();
        buffer_write(UART2_tx_buffer, string[currentByte++]);
        end_critical_section();
    }

    UART2_TX_IE_enable();
}

void UART2_tx_char(char data) {
    if (buffer_is_full(UART2_tx_buffer)) {
        UART2_TX_IE_enable();
        delay_ms(10);
    }

    begin_critical_section();
    buffer_write(UART2_tx_buffer, data);
    end_critical_section();

    UART2_TX_IE_enable();
}

/* -------------------------------------------------------------------------- */
// UART2 receive

/*  Notes on UART2_rx_ISR()

    This function is an Interrupt Vector Table compatible ISR to respond to the
    U2RX interrupt signal. This signal is generated whenever there is an unread
    byte in U2RXB.
*/

void __interrupt(irq(U2RX), high_priority) UART2_rx_ISR() {
    buffer_write(UART2_rx_buffer, U2RXB);
}

char UART2_rx_char(void) {
    if (buffer_is_empty(UART2_rx_buffer))
        return 0;

    begin_critical_section();
    char data = buffer_read(UART2_rx_buffer);
    end_critical_section();

    return data;
}

/* ************************************************************************** */
// UART1 tests

void UART1_tx_buffer_overflow_test(void) {
    // TODO: write me
}