class Pin:
    # possible tags
    tags = [
        'input',
        'output',
        'tristate',
        'gpio',
        'analog',
        'pullup',
        'button',
        'pps',
    ]

    # shortcuts
    button = ['input', 'gpio', 'button']
    relay = ['output', 'gpio', 'tristate']
    uart_tx = ['output', 'pps']
    uart_rx = ['input', 'pps']
    analog_in = ['input', 'analog']
    led = ['output', 'gpio']
    freq = ['input', 'gpio', 'pps']


common = {
    'A0': ('FWD_PIN', Pin.analog_in),
    'A1': ('REV_PIN', Pin.analog_in),
    'A2': ('POWER_LED_PIN', Pin.led),
    'A3': ('POWER_BUTTON_PIN', Pin.button),
    'A4': ('CDN_BUTTON_PIN', Pin.button),
    'A5': ('LUP_BUTTON_PIN', Pin.button),
    'A6': ('FP_CLOCK_PIN', Pin.led),
    'A7': ('RADIO_CMD_PIN', ['output', 'gpio']),
    #
    'B0': None,
    'B1': ('ANT_LED_PIN', Pin.led),
    'B2': ('CUP_BUTTON_PIN', Pin.button),
    'B3': None,
    'B4': ('FUNC_BUTTON_PIN', Pin.button),
    'B5': ('LDN_BUTTON_PIN', Pin.button),
    'B6': None,
    'B7': None,
    #
    'C0': ('RELAY_CLOCK_PIN', Pin.led),
    'C1': ('RELAY_DATA_PIN', Pin.led),
    'C2': ('RELAY_STROBE_PIN', Pin.led),
    'C3': ('BYPASS_LED_PIN', Pin.led),
    'C4': ('FP_STROBE_PIN', Pin.led),
    'C5': ('FP_DATA_PIN', Pin.led),
    'C6': None,
    'C7': None,
}


development = {
    'B6': ('DEBUG_RX_PIN', Pin.uart_rx),
    'B7': ('DEBUG_TX_PIN', Pin.uart_tx),
    #
    'F0': ('FREQ_PIN', Pin.freq),
    'F1': ('ANT_BUTTON_PIN', Pin.button),
    'F2': ('TUNE_BUTTON_PIN', Pin.button),
    #
    'F6': ('USB_TX_PIN', Pin.uart_tx),
    'F7': ('USB_RX_PIN', Pin.uart_rx),
}


release = {
    'B6': ('ANT_BUTTON_PIN', Pin.button),
    'B7': ('TUNE_BUTTON_PIN', Pin.button),
    #
    'E3': ('FREQ_PIN', Pin.freq),
}
