#include "includes.h"
#include "shell.h"
#include "uart.h"
#include "pps.h"

/* ************************************************************************** */

// ASCII helper macros
#define SHELL_ASCII_NUL				'\0'
#define SHELL_ASCII_BEL				'\a'
#define SHELL_ASCII_BS				'\b'
#define SHELL_ASCII_HT				'\t'
#define SHELL_ASCII_LF				'\n'
#define SHELL_ASCII_CR				'\r'
#define SHELL_ASCII_ESC				0x1B
#define SHELL_ASCII_DEL				0x7F
#define SHELL_ASCII_US				0x1F
#define SHELL_ASCII_SP				' '
#define SHELL_VT100_ARROWUP			'A'
#define SHELL_VT100_ARROWDOWN		'B'
#define SHELL_VT100_ARROWRIGHT		'C'
#define SHELL_VT100_ARROWLEFT		'D'

/* ************************************************************************** */

#define SHELL_VERSION_STRING "\n\ruShell 1.0.1"
#define SHELL_PROMPT_STRING "device>"

// Defines the maximum number of commands that can be registered
#define CONFIG_SHELL_MAX_COMMANDS 20

// Defines the maximum characters that the input buffer can accept
#define CONFIG_SHELL_MAX_INPUT 70

// Configures the maximum number of arguments per command tha can be accepted
#define CONFIG_SHELL_MAX_COMMAND_ARGS 10

// Defines the buffer for formatted string output from program memory
#define CONFIG_SHELL_FMT_BUFFER 70

/* ************************************************************************** */

// This structure holds the data for every command registered on the shell
struct shell_command_entry {
	shell_program_t shell_program;
	const char * shell_command_string;
};

/*	This structure array contains the available commands and they associated
  	function entry point, other data required by the commands may be added to
  	this structure
*/
struct shell_command_entry list[CONFIG_SHELL_MAX_COMMANDS];

/* 	This array of pointers to characters holds the addresses of the begining of
  	the parameter strings passed to the programs
*/
char *argv_list[CONFIG_SHELL_MAX_COMMAND_ARGS];

char shell_rx_buffer[CONFIG_SHELL_MAX_INPUT];

bool initialized = false;

/* ************************************************************************** */

// forward declarations
static int shell_parse(char * buf, char** argv, unsigned short maxargs);
int shell_help(int argc, char** argv);
int shell_test(int argc, char** argv);

/* ************************************************************************** */

static void shell_prompt()
{
	shell_print((const char *) SHELL_PROMPT_STRING);
}

void shell_init(void)
{
	// PPS Setup
    U2RXPPS = (PPS_PORT_D & PPS_PIN_3);
	RD2PPS = PPS_UART2_TX;

	UART2_init(_115200);

	for (uint8_t i = 0; i < CONFIG_SHELL_MAX_COMMANDS; i++) {
		list[i].shell_program = 0;
		list[i].shell_command_string = 0;
	}
	shell_register(shell_help, "help");
	shell_register(shell_test, "test");

	initialized = true;

	shell_println((const char *) SHELL_VERSION_STRING);

	shell_prompt();
}

bool shell_register(shell_program_t program, const char *string)
{
	unsigned char i;

	for (i = 0; i < CONFIG_SHELL_MAX_COMMANDS; i++) {
		if (list[i].shell_program != 0 || list[i].shell_command_string != 0)
			continue;
		list[i].shell_program = program;
		list[i].shell_command_string = string;
		return true;
	}
	return false;
}

/* -------------------------------------------------------------------------- */

static int shell_parse(char * buf, char ** argv, unsigned short maxargs)
{
	int i = 0;
	int argc = 0;
	int length = strlen(buf) + 1; //String length to parse = strlen + 1
	char toggle = 0;

	argv[argc] = &buf[0];

	for (i = 0; i < length && argc < maxargs; i++) {
		switch (buf[i]) {	
			case '\0': // String terminator means at least one arg
				i = length;
				argc++;
				break;
			case '\"': // Check for double quotes for strings as parameters
				if (toggle == 0) {
					toggle = 1;
					buf[i] = '\0';
					argv[argc] = &buf[i + 1];
				} else {
					toggle = 0;
					buf[i] = '\0';
				}
				break;
			case ' ':
				if (toggle == 0) {
					buf[i] = '\0';
					argc++;
					argv[argc] = &buf[i + 1];
				}
				break;
		}
	}
	return argc;
}

void shell_process(void)
{
	unsigned int i = 0;
	unsigned int retval = 0;
	int argc = 0;

	argc = shell_parse(shell_rx_buffer, argv_list, CONFIG_SHELL_MAX_COMMAND_ARGS);
	// sequential search on command table
	for (i = 0; i < CONFIG_SHELL_MAX_COMMANDS; i++) {
		if (list[i].shell_program == 0)
			continue;
		// If string matches one on the list
		if (!strcmp(argv_list[0], list[i].shell_command_string)) {
			// Run the appropiate function
			retval = list[i].shell_program(argc, argv_list);
		}
	}	
}

void shell_task(void)
{
	char rxchar = 0;

	// Number of characters written to buffer (this should be static var)
	static unsigned short count = 0;

	if (!initialized)
		return;

	// Process each one of the received characters
	if (UART2_getc(&rxchar)) {

		switch (rxchar) {
		case SHELL_ASCII_ESC: // For VT100 escape sequences
		case '[':
			// Process escape sequences: maybe later
			break;

		case SHELL_ASCII_DEL:
			UART2_putc(SHELL_ASCII_BEL);
			break;

		case SHELL_ASCII_HT:
			UART2_putc(SHELL_ASCII_BEL);
			break;

		case SHELL_ASCII_CR: // Enter key pressed
			shell_rx_buffer[count] = '\0';
			shell_println("");
			if (count > 0) {
				shell_process();

				count = 0;
			} else {
				shell_println("command not found");
			}
			shell_println("");
			shell_prompt();
			break;

		case SHELL_ASCII_BS: // Backspace pressed
			if (count > 0) {
				count--;
				UART2_putc(SHELL_ASCII_BS);
				UART2_putc(SHELL_ASCII_SP);
				UART2_putc(SHELL_ASCII_BS);
			} else
				UART2_putc(SHELL_ASCII_BEL);
			break;
		default:
			// Process printable characters, but ignore other ASCII chars
			if (count < CONFIG_SHELL_MAX_INPUT && rxchar >= 0x20 && rxchar < 0x7F) {
				shell_rx_buffer[count] = rxchar;
				UART2_putc(rxchar);
				count++;
			}
		}
	}
}

/* -------------------------------------------------------------------------- */

void shell_print_commands(void)
{
	unsigned char i;

	for (i = 0; i < CONFIG_SHELL_MAX_COMMANDS; i++) {
		if (list[i].shell_program != 0 || list[i].shell_command_string != 0) {
			shell_println(list[i].shell_command_string);
		}
	}
}

int shell_help(int argc, char** argv)
{
	shell_print_commands();

	return SHELL_RET_SUCCESS;
}

int shell_test(int argc, char** argv)
{
	print_str_ln("-----------------------------------------------");
	print_str_ln("SHELL DEBUG / TEST UTILITY");
	print_str_ln("-----------------------------------------------");
	print_str_ln("");
	print_cat("Received ", argc);
	print_str_ln(" arguments for test command\r\n");

	// Print each argument with string lenghts
	for(uint8_t i = 0; i < argc; i++)
	{
		// Print formatted text to terminal
		// shell_printf("%d - \"%s\" [len:%d]\r\n", i, argv[i], strlen(argv[i]) );
		print_str_ln(argv[i]);
	}

	return SHELL_RET_SUCCESS;
}

/* -------------------------------------------------------------------------- */

// wrap the UART driver so we don't have to repeat the terminator 30 times
void shell_tx_string(const char *string)
{
    UART2_tx_string(string, '\0');
}

void shell_print(const char *string)
{
	shell_tx_string(string);
}

void shell_println(const char *string)
{
	shell_print(string);
	shell_print((const char *) "\r\n");
}


