#ifndef _SHELL_KEYCODES_H_
#define _SHELL_KEYCODES_H_

/* ************************************************************************** */
// Control characters: AKA 1 byte long sequences

// ctrl + some key
#define KEY_CTRL_C 3
#define KEY_CTRL_D 4
#define KEY_CTRL_E 5
#define KEY_CTRL_K 11
#define KEY_CTRL_U 21
#define KEY_CTRL_Y 25
#define KEY_CTRL_Z 26
#define KEY_CTRL_CR 10

#define KEY_BS 8  // backspace
#define KEY_HT 9  // tab
#define KEY_LF 10 // Line Feed
#define KEY_CR 13 // Carriage Return

#define KEY_TLD 126 // ~
#define KEY_BRKT 91

/* ************************************************************************** */

// escape sequence codes
#define KEY_ESC 27

// modifier keys?
#define MOD_SHIFT 50
#define MOD_ALT 51
#define MOD_CTRL 53

/* -------------------------------------------------------------------------- */

// alt + backspace (27 127)
#define KEY_ALT_BS 127

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
// home (27 91 49 126)
// ^home (27 91 49 59 53 72)
#define KEY_HOME 72
#define KEY_HOME2 49

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

// F5 (27 91 49 54 126)
#define KEY_F5 54
#define KEY_F5_ALT 53
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

#endif