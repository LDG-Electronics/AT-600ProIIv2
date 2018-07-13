#ifndef _SHELL_H_
#define _SHELL_H_

/* ************************************************************************** */

// setup
extern void shell_init(void);

/*	Main Shell processing 
 	This function implements the main functionality of the command line interface
 	this function should be called frequently so it can handle the input from the
 	data stream.
*/
extern void shell_update(void);

#endif