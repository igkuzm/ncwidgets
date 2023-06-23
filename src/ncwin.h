/**
 * File              : ncwin.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 12.06.2023
 * Last Modified Date: 14.06.2023
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#ifndef NC_WIN_H
#define NC_WIN_H

#include <curses.h>
#include <panel.h>
#include <stdbool.h>
#include "utils.h"

typedef struct ncwin {
	PANEL *parent;
	PANEL *spanel;
	PANEL *panel;
	WINDOW *overlay;
	WINDOW *shadow;
	int color;
	u8char_t *title;
} ncwin_t;

ncwin_t *
nc_window_new(
		PANEL *parent,
		const char *title,
		int h, int w, int y, int x,
		int color,
		bool box,
		bool shadow
		);

int
nc_window_move(
		ncwin_t *ncwin,
		int y, int x
		);

void nc_window_set_title(ncwin_t *ncwin, const char *title);
int nc_window_hide(ncwin_t *ncwin);
int nc_window_show(ncwin_t *ncwin);
bool nc_window_hidden(ncwin_t *ncwin);
int nc_window_activate(ncwin_t *ncwin);
void nc_window_destroy(ncwin_t *ncwin);

#endif /* ifndef NC_WIN_H */
