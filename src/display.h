#ifndef _DISPLAY_H_
#define _DISPLAY_H_

/* ************************************************************************** */

extern void display_init(void);

/* -------------------------------------------------------------------------- */

#define led_on(); ANT_LED = 0;
#define led_off(); ANT_LED = 1;

/* -------------------------------------------------------------------------- */

extern void bargraph_off(void);
extern void bargraph_red(void);
extern void bargraph_blue(void);
extern void bargraph_green(void);

/* -------------------------------------------------------------------------- */

extern void show_startup(void);

#endif