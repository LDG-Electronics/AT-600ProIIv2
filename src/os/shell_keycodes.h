#ifndef _SHELL_KEYCODES_H_
#define _SHELL_KEYCODES_H_

/* ************************************************************************** */

/* define the Key codes */
#define KEY_NUL 0  // ^@ Null character
#define KEY_SOH 1  // ^A Start of heading, = console interrupt
#define KEY_STX 2  // ^B Start of text, maintenance mode on HP console
#define KEY_ETX 3  // ^C End of text
#define KEY_EOT 4  // ^D End of transmission, not the same as ETB
#define KEY_ENQ 5  // ^E Enquiry, goes with ACK; old HP flow control
#define KEY_ACK 6  // ^F Acknowledge, clears ENQ logon hand
#define KEY_BEL 7  // ^G Bell, rings the bell...
#define KEY_BS 8   // ^H Backspace, works on HP terminals/computers
#define KEY_HT 9   // ^I Horizontal tab, move to next tab stop
#define KEY_LF 10  // ^J Line Feed
#define KEY_VT 11  // ^K Vertical tab
#define KEY_FF 12  // ^L Form Feed, page eject
#define KEY_CR 13  // ^M Carriage Return
#define KEY_SO 14  // ^N Shift Out, alternate character set
#define KEY_SI 15  // ^O Shift In, resume defaultn character set
#define KEY_DLE 16 // ^P Data link escape
#define KEY_DC1 17 // ^Q XON, with XOFF to pause listings; "okay to send".
#define KEY_DC2 18 // ^R Device control 2, block-mode flow control
#define KEY_DC3 19 // ^S XOFF, with XON is TERM=18 flow control
#define KEY_DC4 20 // ^T Device control 4
#define KEY_NAK 21 // ^U Negative acknowledge
#define KEY_SYN 22 // ^V Synchronous idle
#define KEY_ETB 23 // ^W End transmission block, not the same as EOT
#define KEY_CAN 24 // ^X Cancel line, MPE echoes !!!
#define KEY_EM 25  // ^Y End of medium, Control-Y interrupt
#define KEY_SUB 26 // ^Z Substitute
#define KEY_ESC 27 // ^[ Escape, next character is not echoed
#define KEY_FS 28  // ^\ File separator
#define KEY_GS 29  // ^] Group separator
#define KEY_RS 30  // ^^ Record separator, block-mode terminator
#define KEY_US 31  // ^_ Unit separator
#define KEY_SP 32  // space character

// Arrow keys
#define KEY_UP 'A'    // Up arrow
#define KEY_DOWN 'B'  // Down arrow
#define KEY_RIGHT 'C' // Right arrow
#define KEY_LEFT 'D'  // Left arrow

// Navigation keys
#define KEY_HOME 'H'
#define KEY_END 'F'
#define KEY_DEL '3'
#define KEY_INS '2'
#define KEY_PGUP 53
#define KEY_PGDN 54

// F keys
#define KEY_FN 79
#define KEY_F1 80
#define KEY_F2 81
#define KEY_F3 82
#define KEY_F4 83

#endif