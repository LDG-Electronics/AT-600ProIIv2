#include "includes.h"
#include "shell.h"
#include "uart.h"
#include "pps.h"
#include "events.h"
#include <string.h>

/* ************************************************************************** */

#define UCMD_CMD_NOT_FOUND 0
#define COMMAND_DELIMITER ';'

typedef void (*command_ptr)(void);

typedef struct Command {
    const char *cmd;
    command_ptr fn;
} Command;

Command command_list[] = {
    { "BP;", read_bypass, },
    { "BP0;", set_bypass_off, },
    { "BP1;", set_bypass_on, },

    { "PK;", read_peak, },
    { "PK0;", set_peak_off, },
    { "PK1;", set_peak_on, },

    { "SC;", read_scale, },
    { "SC0;", set_scale_low, },
    { "SC1;", set_scale_high, },

    { "AT;", read_auto_mode, },
    { "AT0;", set_auto_off, },
    { "AT1;", set_auto_on, },

    { "ZR;", read_hiloz, },
    { "ZR0;", set_low_z, },
    { "ZR1;", set_high_z, },

    { "AN;", read_antenna, },
    { "AN0;", set_antenna_two, },
    { "AN1;", set_antenna_one, },

    { "FT;", request_full_tune, },
    { "MT;", request_memory_tune, },

    { "RL;", read_relays, },
    { "CU;", capacitor_increment, },
    { "CD;", capacitor_decrement, },
    { "LU;", capacitor_increment, },
    { "LD;", inductor_decrement, },
};

/* ************************************************************************** */

volatile shell_flags_s shell_flags;

#define COMMAND_BUFFER_SIZE 32
char command_buffer[COMMAND_BUFFER_SIZE];

/* ************************************************************************** */

void shell_init(void)
{
    // PPS Setup
    U2RXPPS = (PPS_PORT_D & PPS_PIN_3);

    shell_flags.all = 0;

    // initialize the command buffer
    for(uint8_t i = 0; i < COMMAND_BUFFER_SIZE; i++)
    {
        command_buffer[i] = 0;
    }
}

void process_shell_command(void)
{
    uint8_t i = 0;

    while(1)
    {
        if (strcmp(command_buffer, command_list[i].cmd) == 0) 
        {
            command_list[i].fn();
            return;
        }

        i++;
        if(i == 30) break;
    }
    
    print_str_ln("?");
}

void check_for_shell_command(void)
{
    uint8_t i = 0;

    if (shell_flags.delimiterReceived == 1)
    {        
        while(1)
        {
            command_buffer[i] = UART2_rx_char();
            if (command_buffer[i] == '\0') break;
            i++;
        }

        process_shell_command();

        shell_flags.delimiterReceived = 0;
    }
}


