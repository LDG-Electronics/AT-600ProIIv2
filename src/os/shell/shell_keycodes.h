#ifndef _SHELL_KEYCODES_H_
#define _SHELL_KEYCODES_H_

/* ************************************************************************** */
// single code keys

// ctrl + c (3)
#define KEY_ETX 3
// backspace (8)
#define KEY_BS 8
// tab (9)
#define KEY_HT 9

#define KEY_LF 10   // Line Feed
#define KEY_CR 13   // Carriage Return

#define KEY_TLD 126 // ~
#define KEY_BRKT 91

// escape sequence codes
#define KEY_ESC 27

/* -------------------------------------------------------------------------- */
// Arrow keys

// up arrow (27 91 65)
#define KEY_UP 65
// down arrow (27 91 66)
#define KEY_DOWN 66

// right arrow (27 91 67)
// ^right arrow (27 91 49 59 53 67)
#define KEY_RIGHT 67
// left arrow (27 91 68)
// ^left arrow (27 91 49 59 53 68)
#define KEY_LEFT 68

/* -------------------------------------------------------------------------- */
// Navigation keys

// home (27 91 72)
// ^home (27 91 49 59 53 72)
#define KEY_HOME 72

// end (27 91 70)
// ^end (27 91 49 59 53 70)
#define KEY_END 70

// delete (27 91 51 126)
// ^delete (27 91 51 59 53 126)
#define KEY_DEL 51

// insert (27 91 50 126)
// ^insert (27 91 50 59 53 126)
#define KEY_INS 50

// page up (27 91 53 126)
#define KEY_PGUP 53
// page down (27 91 54 126)
#define KEY_PGDN 54

/* -------------------------------------------------------------------------- */
// F keys

// F1 (27 79 80)
#define KEY_F1 80
// F2 (27 79 81)
#define KEY_F2 81
// F3 (27 79 82)
#define KEY_F3 82
// F4 (27 79 83)
#define KEY_F4 83

// F5 (27 91 49 53 126)
#define KEY_F5 53
// F6 (27 91 49 55 126)
#define KEY_F6 55
// F7 (27 91 49 56 126)
#define KEY_F7 56
// F8 (27 91 49 57 126)
#define KEY_F8 57

// F9 (27 91 50 48 126)
#define KEY_F9 48
// F10 (27 91 50 49 126)
#define KEY_F10 49
// F11 (27 91 50 51 126)
#define KEY_F11 51
// F12 (27 91 50 52 126)
#define KEY_F12 52

/* -------------------------------------------------------------------------- */

// unused keycodes
#define KEY_NUL 0  // ^@ Null character
#define KEY_SOH 1  // ^A Start of heading, = console interrupt
#define KEY_STX 2  // ^B Start of text, maintenance mode on HP console
#define KEY_EOT 4  // ^D End of transmission, not the same as ETB
#define KEY_ENQ 5  // ^E Enquiry, goes with ACK; old HP flow control
#define KEY_ACK 6  // ^F Acknowledge, clears ENQ logon hand
#define KEY_BEL 7  // ^G Bell, rings the bell...
#define KEY_VT 11  // ^K Vertical tab
#define KEY_FF 12  // ^L Form Feed, page eject
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
#define KEY_FS 28  // ^\ File separator
#define KEY_GS 29  // ^] Group separator
#define KEY_RS 30  // ^^ Record separator, block-mode terminator
#define KEY_US 31  // ^_ Unit separator
#define KEY_SP 32  // space character

#endif