/**
 * File              : ncbutton.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 14.06.2023
 * Last Modified Date: 16.06.2023
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#include "ncbutton.h"
#include "utils.h"
#include "types.h"
#include "keys.h"
#include <curses.h>
#include <ncurses.h>

void nc_button_refresh(ncbutton_t *ncbutton){
	int h, w, y, x;
	getmaxyx(ncbutton->ncwin->overlay, h, w);

	attr_t focused = A_NORMAL;
	if (ncbutton->focused)
		focused = A_REVERSE;

	// fill with blank 
	for (y = 0; y < h - 2; ++y)
		for (x = 0; x < w - 2; x++)
			mvwaddch(ncbutton->ncwin->overlay, y+1, x+1, ' '| focused);
	
	//fill with data
	for (y = 0; y < h - 2 && y < ncbutton->lines; ++y) {
		wmove(ncbutton->ncwin->overlay, y + 1, 1);		
		u8char_t *str = ncbutton->info[y]; 
		
		for (x = 0; x < w - 2 && str[x].utf8[0]; ++x) {
			wattron (ncbutton->ncwin->overlay, str[x].attr  | focused);
			waddstr (ncbutton->ncwin->overlay, str[x].utf8);	
			wattroff(ncbutton->ncwin->overlay, str[x].attr  | focused);
		}
	}

	wrefresh(ncbutton->ncwin->overlay);
}

ncbutton_t * nc_button_new(
		PANEL *parent,
		int h, int w, int y, int x,
		int color,
		const char **value,
		int lines,
		bool box,
		bool shadow
		)
{
	ncbutton_t *ncbutton = malloc(sizeof(ncbutton_t));
	if (!ncbutton)
		return NULL;

	ncbutton->info = malloc( 8 * lines + 8);
	if (!ncbutton->info){
		free(ncbutton);
		return NULL;
	}
	ncbutton->lines = lines;
	ncbutton->focused = false;
	
	/* copy values */
	int i, maxlen = 0;
	for (i = 0; i < ncbutton->lines; ++i) {
		ncbutton->info[i] = str2ucharstr(value[i], color);
		int len = ucharstrlen(ncbutton->info[i]);
		if (len > maxlen)
			maxlen = len;
	}

	ncbutton->ncwin = nc_window_new(parent, NULL, h, w, y, x, color, box, shadow);
	if (!ncbutton->ncwin){
		for (i = 0; i < ncbutton->lines; ++i) {
			free(ncbutton->info[i]);
		}
		free(ncbutton->info);
		free(ncbutton);
		return NULL;
	}

	nc_button_refresh(ncbutton);
	
	return ncbutton;
}


void nc_button_destroy(ncbutton_t *ncbutton)
{
	nc_window_destroy(ncbutton->ncwin);
	int i;
	for (i = 0; i < ncbutton->lines; ++i) {
		free(ncbutton->info[i]);
	}
	free(ncbutton->info);
	free(ncbutton);
}

void nc_button_set_focused(ncbutton_t *ncbutton, bool focused)
{
	ncbutton->focused = focused;
	nc_button_refresh(ncbutton);
}

void nc_button_activate(
		ncbutton_t *ncbutton,
		void *userdata,
		int (*callback)(void *userdata, enum SCREEN type, void *object, chtype key)		
		)
{
	nc_button_set_focused(ncbutton, true);

	chtype ch;
	while (ch != CTRL('x')) {
		ch = getch();
		// stop execution if callback not NULL
		if (callback)
			if(callback(userdata, SCREEN_ncbutton, ncbutton, ch))
				break;

		//switch keys
		switch (ch) {
			case KEY_MOUSE:
				{
					MEVENT event;
					if (getmouse(&event) == OK) {
						if (wenclose(ncbutton->ncwin->overlay, event.y, event.x)){
							if (event.bstate & BUTTON1_PRESSED){
								if (callback)
									if(callback(userdata, SCREEN_ncbutton, ncbutton, KEY_RETURN))
										return;
							}
						}
					}
					break;
				}

			default:
				beep();
				break;
		}
	}
}
