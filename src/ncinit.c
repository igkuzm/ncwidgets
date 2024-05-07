/**
 * File              : nc_init.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 08.05.2024
 * Last Modified Date: 08.05.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#include "ncwidgets.h"
#include <locale.h>

void nc_init(
		const char *locale,
		int color
		)
{
	if (!locale)
		locale = "";
	setlocale(LC_ALL, locale);

	#undef  NCURSES_MOUSE_VERSION
	#define NCURSES_MOUSE_VERSION 2

	initscr();

	/* init colosrs */
	start_color();	
	init_colors();
	bkgd(COLOR_PAIR(color));	

	/* init mouse */
	mouseinterval(0);
	mousemask(ALL_MOUSE_EVENTS, NULL);

	/* init Fn keys */
	keypad(stdscr, TRUE);	
	
	/* hide cursor */
	curs_set(0);	

	/* setup curses */
	cbreak();  //Не использовать буфер для функции getch()
	raw();
	nonl();
	noecho();	
}

void nc_quit()
{
	endwin();
}
