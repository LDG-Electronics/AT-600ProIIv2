#include "../includes.h"

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

/* ************************************************************************** */

// forward declarations
static int shell_parse(char * buf, char** argv, unsigned short maxargs);
/* ************************************************************************** */

void shell_prompt(void)
{
	print(SHELL_PROMPT_STRING);
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

	println(SHELL_VERSION_STRING);

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

void shell_update(void)
{
	char rxchar = 0;

	// Number of characters written to buffer (this should be static var)
	static unsigned short count = 0;

	// Process each one of the received characters
	if (UART2_getc(&rxchar)) {

		switch (rxchar) {
		case SHELL_ASCII_ESC: // For VT100 escape sequences
		case '[':
			// Process escape sequences: maybe later
			break;

		case SHELL_ASCII_DEL:
			putch(SHELL_ASCII_BEL);
			break;

		case SHELL_ASCII_HT:
			putch(SHELL_ASCII_BEL);
			break;

		case SHELL_ASCII_CR: // Enter key pressed
			shell_rx_buffer[count] = '\0';
			println("");
			if (count > 0) {
				shell_process();

				count = 0;
			} else {
				println("command not found");
			}
			println("");
			shell_prompt();
			break;

		case SHELL_ASCII_BS: // Backspace pressed
			if (count > 0) {
				count--;
				putch(SHELL_ASCII_BS);
				putch(SHELL_ASCII_SP);
				putch(SHELL_ASCII_BS);
			} else
				putch(SHELL_ASCII_BEL);
			break;
		default:
			// Process printable characters, but ignore other ASCII chars
			if (count < CONFIG_SHELL_MAX_INPUT && rxchar >= 0x20 && rxchar < 0x7F) {
				shell_rx_buffer[count] = rxchar;
				putch(rxchar);
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
			println(list[i].shell_command_string);
		}
	}
}

