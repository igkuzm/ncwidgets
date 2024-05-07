/**
 * File              : ncwidgets.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 12.06.2023
 * Last Modified Date: 08.05.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#include "ncwidgets.h"
#include "stuctures.h"
#include <curses.h>
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

void nc_widget_refresh(NcWidget *widget){
	widget->on_refresh(widget);
}
void nc_widget_set_focused(NcWidget *widget, bool focused){
	widget->on_set_focused(widget, focused);
}
void nc_widget_activate(
		NcWidget *widget, 
		void *userdata,
		NCRET callback(NcWidget *widget, void *userdata, chtype ch)
		)
{
	widget->on_activate(widget, userdata, callback);
}
void nc_widget_destroy(NcWidget *widget){
	widget->on_destroy(widget);
}

int nc_widget_move(NcWidget *widget, int y, int x){
	return nc_win_move(widget->ncwin, y, x);
}

int nc_widget_resize(NcWidget *widget, int h, int w){
	int ret = nc_win_resize(widget->ncwin, h, w);
	nc_widget_refresh(widget);
	return ret;
}
