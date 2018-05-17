#include "includes.h"

/* ************************************************************************** */
// The meter update can contain zeros, so the terminator has to be something
// other than null.
#define METER_COMMS_TERMINATOR '\e'
// Apparently, the magic string for syncing is 15 zeros plus AzAz. This was
// chosen by Russ during the High-Power Meter development, and can't be changed.
#define METER_SYNC_STRING "000000000000000AzAz"

uint8_t meter_update_status = 0;
char meter_update_buffer[9];

void meter_init(void)
{
    // PPS Setup
    U1RXPPS = (PPS_PORT_C & PPS_PIN_7);
    RC6PPS = PPS_UART1_TX;

    UART1_init(_38400);

    // The last three bytes of the buffer never change, so init them here
    meter_update_buffer[6] = ';';
    meter_update_buffer[7] = ';';
    meter_update_buffer[8] = METER_COMMS_TERMINATOR;
}

/* -------------------------------------------------------------------------- */

void check_for_meter_sync(void)
{
    char data = UART1_rx_char();

    if (data == '\0') {
        return;
    } else if (data == 'Z') {
        UART1_tx_string(METER_SYNC_STRING, '\0');
    } else if (data == 'S') {
        meter_update_status = 1;
    } else if (data == 'X') {
        meter_update_status = 0;
    }
}

void send_meter_update(void)
{
    uint16_t tempFWD = adc_measure(0) >> 2;
    uint16_t tempREV = adc_measure(1) >> 2;
    uint16_t tempPeriod = get_period();

    meter_update_buffer[0] = (uint8_t)(tempFWD >> 8);
    meter_update_buffer[1] = (uint8_t)(tempFWD & 0xff);
    meter_update_buffer[2] = (uint8_t)(tempREV >> 8);
    meter_update_buffer[3] = (uint8_t)(tempREV & 0xff);
    meter_update_buffer[4] = (uint8_t)(tempPeriod >> 8);
    meter_update_buffer[5] = (uint8_t)(tempPeriod & 0xff);

    UART1_tx_string(meter_update_buffer, METER_COMMS_TERMINATOR);
}

#define METER_UPDATE_INTERVAL 100
void attempt_meter_update(void)
{
    static uint24_t nextUpdateTime = 0;
    
    check_for_meter_sync();

    if (meter_update_status == 1) 
    {
        uint24_t currentTime = systick_read();

        if(currentTime >= nextUpdateTime)
        {
            send_meter_update();
            
            nextUpdateTime = currentTime + METER_UPDATE_INTERVAL;
        }
    }
}