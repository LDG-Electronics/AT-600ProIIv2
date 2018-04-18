#include "includes.h"
#include "uart.h"
#include "hardware.h"

/* ************************************************************************** */
// Common UART settings

/* -------------------------------------------------------------------------- */
// Circular buffer tools
/*  If BUFFER_SIZE is 256, then the head and tail indexes become significantly
    easier to manage. Bounds checking and modulus operations can be replaced
    a single postincrement(x++) instruction, because a uint8_t will wraparound
    from 0xff to 0x00 on its own.

    Yes, this wastes RAM. No, it doesn't matter, because this project has a
    minimum of 4k of RAM and nothing else has a significant RAM footprint. If
    memory becomes an issue, the size of these buffers can be reduced. This
    would require rewriting the helper macros to add the previously mentioned
    bounds checking.
*/
#define BUFFER_SIZE 256

typedef struct{
    char contents[BUFFER_SIZE];
    uint8_t head;
    uint8_t tail;
}uart_buffer_s;

#if BUFFER_SIZE == 256
    #define buffer_is_empty(buffer) (buffer.head == buffer.tail)
    #define buffer_is_full(buffer) ((buffer.head + 1) == buffer.tail)
    #define buffer_has_at_least(buffer, space) ((buffer.head + space) >= buffer.tail)
    #define buffer_write(buffer, data) buffer.contents[buffer.head++] = data
    #define buffer_read(buffer) buffer.contents[buffer.tail++]
#else
    #error BUFFER_SIZE other than 256 is not currently supported because the buffer depends on uint8_t overflow behavior
#endif

/* -------------------------------------------------------------------------- */
// UART Baud rate tools

// Baud Rates:                 2400    4800    9600    10417   19200   38400   115200
const uint8_t baudTableH[] = { 0x1A,   0x0D,   0x06,   0x05,   0x03,   0x01,   0x00};
const uint8_t baudTableL[] = { 0x0A,   0x04,   0x82,   0xFF,   0x40,   0xA0,   0x8A};

/* ************************************************************************** */
// UART 1

volatile uart_buffer_s UART1_tx_buffer;
volatile uart_buffer_s UART1_rx_buffer;

void UART1_baud_select(enum baud_rates baudRate)
{
    // Load the selected baud rate into the serial timing registers
    U1BRGH = baudTableH[baudRate];
    U1BRGL = baudTableL[baudRate];
}

void UART1_init(enum baud_rates baudRate)
{
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
void __interrupt(irq(IRQ_U1TX), high_priority) UART1_tx_ISR()
{
    if(buffer_is_empty(UART1_tx_buffer)) {
        PIE3bits.U1TXIE = 0; // disable interrupt
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
void UART1_tx_string(const char *string, const char terminator)
{
    uint16_t totalBytes = 0;
    uint16_t currentByte = 0;
    uint16_t remainingBytes = 0;
    
    // loop until hitting null
    while(string[currentByte] != terminator)
    {
        // buffer overflow handler
        if (buffer_is_full(UART1_tx_buffer))
        {
            PIE3bits.U1TXIE = 1;

            // Find out how long the string is
            if(totalBytes == 0){
                while(string[totalBytes++] != terminator); 
            }
            remainingBytes = totalBytes - currentByte;

            // TODO: test me
            if (remainingBytes < BUFFER_SIZE) {
                // Block until there's enough room for the rest of the string
                while(!buffer_has_at_least(UART1_tx_buffer, remainingBytes));
            } else if (remainingBytes >= BUFFER_SIZE) {
                // Block until the buffer is almost empty
                while(!buffer_has_at_least(UART1_tx_buffer, 250));
            }
        }

        begin_critical_section();
        buffer_write(UART1_tx_buffer, string[currentByte++]);
        end_critical_section();
    }

    PIE3bits.U1TXIE = 1;
}

/* -------------------------------------------------------------------------- */
// UART1 receive

/*  Notes on UART1_rx_ISR()

    This function is an Interrupt Vector Table compatible ISR to respond to the
    IRQ_U1RX interrupt signal. This signal is generated whenever there is an
    unread byte in U1RXB. 
*/

void __interrupt(irq(IRQ_U1RX), high_priority) UART1_rx_ISR()
{
    buffer_write(UART1_rx_buffer, U1RXB);
}

char UART1_rx_char(void)
{
    char data = 0;

    if(buffer_is_empty(UART1_rx_buffer)) return '\0';

    begin_critical_section();
    data = buffer_read(UART1_rx_buffer);
    end_critical_section();

    return data;
}

/* ************************************************************************** */
// UART 2

volatile uart_buffer_s UART2_tx_buffer;
volatile uart_buffer_s UART2_rx_buffer;

void UART2_baud_select(enum baud_rates baudRate)
{
    // Load the selected baud rate into the serial timing registers
    U2BRGH = baudTableH[baudRate];
    U2BRGL = baudTableL[baudRate];
}

void UART2_init(enum baud_rates baudRate)
{
    U2CON0bits.BRGS = 1; // Baud Rate is set to high speed
    U2CON0bits.TXEN = 1; // Transmit is enabled
    U2CON0bits.RXEN = 1; // Recieve is enabled

    UART2_baud_select(baudRate);

    // initialize ring buffer pointers
    UART2_tx_buffer.head = 0;
    UART2_tx_buffer.tail = 0;
    UART2_rx_buffer.head = 0;
    UART2_rx_buffer.tail = 0;

    U2CON1bits.ON = 1; // Enable UART1
}

/* -------------------------------------------------------------------------- */
// UART2 transmit

/*  Notes on UART2_tx_ISR()

    This function is an Interrupt Vector Table compatible ISR to respond to the
    IRQ_U2TX interrupt signal. This signal is generated whenever U2TXB is empty
    and PIE3bits.U2TXIE is enabled. In effect, the interrupt flag is set every
    time UART2 finishes transmitting a byte.

*/
void __interrupt(irq(IRQ_U2TX), high_priority) UART2_tx_ISR()
{
    if(buffer_is_empty(UART2_tx_buffer)) {
        PIE6bits.U2TXIE = 0; // disable interrupt
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
void UART2_tx_string(const char *string, const char terminator)
{
    uint16_t totalBytes = 0;
    uint16_t currentByte = 0;
    uint16_t remainingBytes = 0;
    
    // loop until hitting null
    while(string[currentByte] != terminator)
    {
        // buffer overflow handler
        if (buffer_is_full(UART2_tx_buffer))
        {
            PIE6bits.U2TXIE = 1;

            // Find out how long the string is
            if(totalBytes == 0){
                while(string[totalBytes++] != terminator); 
            }
            remainingBytes = totalBytes - currentByte;
            
            // TODO: test me
            if (remainingBytes < BUFFER_SIZE) {
                // Block until there's enough room for the rest of the string
                while(!buffer_has_at_least(UART2_tx_buffer, remainingBytes));
            } else if (remainingBytes >= BUFFER_SIZE) {
                // Block until the buffer is almost empty
                while(!buffer_has_at_least(UART2_tx_buffer, 250));
            }
        }

        begin_critical_section();
        buffer_write(UART2_tx_buffer, string[currentByte++]);
        end_critical_section();
    }

    PIE6bits.U2TXIE = 1;
}

/* -------------------------------------------------------------------------- */
// UART2 receive

/*  Notes on UART2_rx_ISR()

    This function is an Interrupt Vector Table compatible ISR to respond to the
    IRQ_U2RX interrupt signal. This signal is generated whenever there is an
    unread byte in U2RXB. 
*/

void __interrupt(irq(IRQ_U2RX), high_priority) UART2_rx_ISR()
{
    buffer_write(UART2_rx_buffer, U1RXB);
}

char UART2_rx_char(void)
{
    char data = 0;

    if(buffer_is_empty(UART2_rx_buffer)) return '\0';

    begin_critical_section();
    data = buffer_read(UART2_rx_buffer);
    end_critical_section();

    return data;
}

/* ************************************************************************** */
// UART1 tests

void UART1_tx_buffer_overflow(void)
{
    // TODO: write me
}