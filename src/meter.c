#include "meter.h"
#include "os/logging.h"
#include "os/system_time.h"
#include "peripherals/adc.h"
#include "peripherals/uart.h"
static uint8_t LOG_LEVEL = L_SILENT;

/* ************************************************************************** */
// The meter update can contain zeros, so the terminator has to be something
// other than null.
#define METER_COMMS_TERMINATOR '\e'

// Apparently, the magic string for syncing is 15 zeros plus AzAz. This was
// chosen by Russ during the High-Power Meter development, and can't be changed.
#define METER_SYNC_STRING "000000000000000AzAz"

struct {
    char packetBuffer[9];
    unsigned active : 1;
} meter;

/* ************************************************************************** */

EMPTY_UART_INTERFACE(uart);

void meter_init(uart_interface_t interface) {
    //
    uart = interface;

    // The last three bytes of the buffer never change, so init them here
    meter.packetBuffer[6] = ';';
    meter.packetBuffer[7] = ';';
    meter.packetBuffer[8] = METER_COMMS_TERMINATOR;

    log_register();
}

/* -------------------------------------------------------------------------- */

void check_for_meter_sync(void) {
    char data = uart.rx_char();

    if (data == '\0') {
        return;
    } else if (data == 'Z') {
        uart.tx_string(METER_SYNC_STRING, '\0');
    } else if (data == 'S') {
        meter.active = 1;
    } else if (data == 'X') {
        meter.active = 0;
    }
}

void send_meter_update(void) {
    LOG_TRACE({ println("send_meter_update"); });
    uint16_t tempFWD = adc_read(0) >> 2;
    uint16_t tempREV = adc_read(1) >> 2;
    uint16_t tempPeriod = 0xffff;

    meter.packetBuffer[0] = (uint8_t)(tempFWD >> 8);
    meter.packetBuffer[1] = (uint8_t)(tempFWD & 0xff);
    meter.packetBuffer[2] = (uint8_t)(tempREV >> 8);
    meter.packetBuffer[3] = (uint8_t)(tempREV & 0xff);
    meter.packetBuffer[4] = (uint8_t)(tempPeriod >> 8);
    meter.packetBuffer[5] = (uint8_t)(tempPeriod & 0xff);

    uart.tx_string(meter.packetBuffer, METER_COMMS_TERMINATOR);
}

#define METER_UPDATE_INTERVAL 100
void attempt_meter_update(void) {
    check_for_meter_sync();

    if (meter.active == 1) {
        system_time_t currentTime = get_current_time();

        if (time_since(currentTime) >= METER_UPDATE_INTERVAL) {
            send_meter_update();
        }
    }
}