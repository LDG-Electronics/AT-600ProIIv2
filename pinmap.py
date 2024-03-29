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
    # port A
    'A0': ('FWD_PIN', Pin.analog_in),
    'A1': ('REV_PIN', Pin.analog_in),
    'A2': ('POWER_LED_PIN', Pin.led),
    'A3': ('POWER_BUTTON_PIN', Pin.button),
    'A4': ('CDN_BUTTON_PIN', Pin.button),
    'A5': ('LUP_BUTTON_PIN', Pin.button),
    'A6': ('FP_CLOCK_PIN', ['output', 'gpio']),
    'A7': ('RADIO_CMD_PIN', ['output', 'gpio']),

    # port B
    'B0': None,
    'B1': ('ANT_LED_PIN', Pin.led),
    'B2': ('CUP_BUTTON_PIN', Pin.button),
    'B3': None,
    'B4': ('FUNC_BUTTON_PIN', Pin.button),
    'B5': ('LDN_BUTTON_PIN', Pin.button),
    'B6': None,
    'B7': None,

    # port C
    'C0': ('RELAY_CLOCK_PIN', Pin.led),
    'C1': ('RELAY_DATA_PIN', Pin.led),
    'C2': ('RELAY_STROBE_PIN', Pin.led),
    'C3': ('BYPASS_LED_PIN', Pin.relay),
    'C4': ('FP_STROBE_PIN', Pin.relay),
    'C5': ('FP_DATA_PIN', Pin.relay),
    'C6': ('USB_TX_PIN', Pin.uart_tx),
    'C7': ('USB_RX_PIN', Pin.uart_rx),

    # port E
    'E0': ('FREQ_PIN', Pin.freq),
}


development = {
    'D2': ('DEBUG_TX_PIN', Pin.uart_tx),
    'D3': ('DEBUG_RX_PIN', Pin.uart_rx),
    'E1': ('TUNE_BUTTON_PIN', Pin.button),
    'E2': ('ANT_BUTTON_PIN', Pin.button),
}


release = {
    'B6': ('TUNE_BUTTON_PIN', Pin.button),
    'B7': ('ANT_BUTTON_PIN', Pin.button),
}