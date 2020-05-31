#!/usr/bin/env python3

import os
import sys
import json

# ------------------------------------------------------------------------------

adc_channel_lookup = {"F7": "47",
                      "F6": "46",
                      "F5": "45",
                      "F4": "44",
                      "F3": "43",
                      "F2": "42",
                      "F1": "41",
                      "F0": "40",

                      "E2": "34",
                      "E1": "33",
                      "E0": "32",

                      "D7": "31",
                      "D6": "30",
                      "D5": "29",
                      "D4": "28",
                      "D3": "27",
                      "D2": "26",
                      "D1": "25",
                      "D0": "24",

                      "C7": "23",
                      "C6": "22",
                      "C5": "21",
                      "C4": "20",
                      "C3": "19",
                      "C2": "18",
                      "C1": "17",
                      "C0": "16",

                      "B7": "15",
                      "B6": "14",
                      "B5": "13",
                      "B4": "12",
                      "B3": "11",
                      "B2": "10",
                      "B1": "9",
                      "B0": "8",

                      "A7": "7",
                      "A6": "6",
                      "A5": "5",
                      "A4": "4",
                      "A3": "3",
                      "A2": "2",
                      "A1": "1",
                      "A0": "0"}


def adc_channel(pin):
    channel = pin["port"] + pin["pin"]
    return adc_channel_lookup[channel]


def GPIO_read_function_signature(pin_name):
    return "bool " + "read_" + pin_name + "(void)"


def GPIO_write_function_signature(pin_name):
    return "void " + "set_" + pin_name + "(bool value)"


# ------------------------------------------------------------------------------

def pin_declarations():
    pin_list = load_pins_from_file()

    text = "\n"

    text += "// GPIO read functions \n"
    for pin in [p for p in pin_list if p["usage"]["gpio"] and p["direction"]["input"]]:
        text += "extern " + GPIO_read_function_signature(pin["name"]) + ";\n"
    text += "\n"

    text += "// GPIO write functions \n"
    for pin in [p for p in pin_list if p["usage"]["gpio"] and p["direction"]["output"]]:
        text += "extern " + GPIO_write_function_signature(pin["name"]) + ";\n"
    text += "\n"

    text += "// PPS initialization macros\n"
    for pin in [p for p in pin_list if p["usage"]["pps"]]:
        text += "#define PPS_" + pin["name"]

        if pin["direction"]["input"]:
            text += " PPS_INPUT("
        elif pin["direction"]["output"]:
            text += " PPS_OUTPUT("

        text += pin["port"] + ", " + pin["pin"] + ")\n"
    text += "\n"

    text += "// ADC Channel Select macros\n"
    for pin in [p for p in pin_list if p["usage"]["analog"]]:
        text += "#define ADC_" + pin["name"] + " " + adc_channel(pin) + "\n"

    return text


# ------------------------------------------------------------------------------


def pin_definitions():
    pin_list = load_pins_from_file()

    text = "\n"

    text += "// GPIO read functions \n"
    for pin in [p for p in pin_list if p["usage"]["gpio"] and p["direction"]["input"]]:
        text += GPIO_read_function_signature(pin["name"]) + " { "
        text += "return " + "PORT" + pin["port"] + "bits." + "R"
        text += pin["port"] + pin["pin"] + "; }\n"
    text += "\n"

    text += "// GPIO write functions \n"
    for pin in [p for p in pin_list if p["usage"]["gpio"] and p["direction"]["output"]]:
        text += GPIO_write_function_signature(pin["name"]) + " { "
        text += "LAT" + pin["port"] + "bits." + "LAT"
        text += pin["port"] + pin["pin"] + " = " + "value; }\n"

    return text


def pins_init():
    pin_list = load_pins_from_file()

    text = "\n"

    text += "void pins_init(void) {\n"
    for pin in pin_list:
        text += "    // " + pin["name"] + "\n"
        for register in pin["registers"]:
            text += "    "
            text += register + pin["port"] + "bits."
            text += register + pin["port"] + pin["pin"] + " = "
            text += pin["registers"][register] + ";" + "\n"

        text += "\n"
    text += "}\n"

    return text


# ------------------------------------------------------------------------------


def load_pins_from_file():
    return json.loads(open('pins.json').read())
