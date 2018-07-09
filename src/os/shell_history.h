#ifndef _SHELL_HISTORY_H_
#define _SHELL_HISTORY_H_

/* ************************************************************************** */

// extern line_t history[SHELL_HISTORY_LENGTH];

/* ************************************************************************** */

extern void reset_history_line(uint8_t line);
extern void copy_current_line_to_history(uint8_t line);
extern void copy_current_line_from_history(uint8_t line);

#endif