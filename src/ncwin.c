/**
 * File              : ncwin.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 12.06.2023
 * Last Modified Date: 08.05.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#include "struct.h"
#include <curses.h>
#include <ncurses.h>
#include <panel.h>
#include <stdlib.h>
#include "colors.h"
#include "utils.h"

void nc_win_set_title(NcWin *ncwin, const char *title)
{
	if (ncwin->title)
		free(ncwin->title);
	ncwin->title = str2ucharstr(title, ncwin->color); 
	wmove(ncwin->overlay, 0, 1);
	int i = 0;
	while (ncwin->title[i].utf8[0]){
		wattron (ncwin->overlay, ncwin->title[i].attr);
		waddstr (ncwin->overlay, ncwin->title[i].utf8);	
		wattroff(ncwin->overlay, ncwin->title[i].attr);
		i++;
	}
	wrefresh(ncwin->overlay);
}

NcWin *
nc_win_new(
		NcWin *parent,
		const char *title, 
		int h, int w, int y, int x, 
		int color,
		bool box, 
		bool shadow) 
{
	int i;
	
	NcWin *ncwin = malloc(sizeof(NcWin));
	if (!ncwin)
		return NULL;

	ncwin->parent = parent;
	ncwin->shadow = NULL;
	ncwin->panel  = NULL;
	ncwin->spanel = NULL;
	ncwin->color  = color; 
	ncwin->title  = NULL;
	ncwin->box    = box;
	
	if (shadow){
		if (parent){
			ncwin->shadow = derwin(parent->panel->win, h, w, y + 1, x + 2);	
		} else {
			ncwin->shadow = newwin(h, w, y + 1, x + 2);	
			ncwin->spanel = new_panel(ncwin->shadow);
		}
		wbkgd(ncwin->shadow, COLOR_PAIR(BLACK_ON_BLACK));
		wrefresh(ncwin->shadow);
	}

	if (parent)
		ncwin->overlay = derwin(parent->panel->win, h, w, y, x);
	else
		ncwin->overlay = newwin(h, w, y, x);
	if (!ncwin->overlay){
		free(ncwin);	
		return NULL;
	}
	wbkgd(ncwin->overlay, COLOR_PAIR(color));

	if (box)
		box(ncwin->overlay, 0, 0);
	
	if (title){
		nc_win_set_title(ncwin, title);
	}

	if (!parent)
		ncwin->panel = new_panel(ncwin->overlay);

	update_panels();
	doupdate();

	return ncwin;
}

int nc_win_move(NcWin *ncwin, int y, int x){
	int ret;
	if (ncwin->parent)
		return -1;
	
	if (ncwin->spanel)
		ret = move_panel(ncwin->spanel, y + 1, x + 2);
	
	ret = move_panel(ncwin->panel, y, x);
	
	update_panels();
	doupdate();

	return ret;
}

int nc_win_resize(NcWin *ncwin, int h, int w){
	int ret;
	if (ncwin->parent)
		return -1;
	
	wresize(ncwin->overlay, h, w);
	
	if (ncwin->shadow)
		wresize(ncwin->shadow, h, w);
	
	if (ncwin->box)
		box(ncwin->overlay, 0, 0);

	update_panels();
	doupdate();

	return ret;
}

int nc_win_hide(NcWin *ncwin)
{
	if (ncwin->parent)
		return -1;

	if (ncwin->spanel)
		hide_panel(ncwin->spanel);
	int ret = hide_panel(ncwin->panel);
	update_panels();
	doupdate();
	return ret;
}

int nc_win_show(NcWin *ncwin)
{
	if (ncwin->parent)
		return -1;
	
	if (ncwin->spanel)
		show_panel(ncwin->spanel);
	int ret =  show_panel(ncwin->panel);
	update_panels();
	doupdate();
	return ret;
}

bool nc_win_hidden(NcWin *ncwin)
{
	if (ncwin->parent)
		return false;

	return panel_hidden(ncwin->panel);
}

int 
nc_win_activate(NcWin *ncwin)
{
	if (ncwin->parent)
		return -1;
	
	if (nc_win_hidden(ncwin))
		nc_win_show(ncwin);

	if (ncwin->shadow)
		top_panel(ncwin->spanel);
	int ret = top_panel(ncwin->panel);
	update_panels();
	doupdate();
	return ret;	
}

void nc_win_destroy(NcWin *ncwin)
{
	NcWin *parent = ncwin->parent;
	werase(ncwin->overlay);
	if (ncwin->shadow)
		werase(ncwin->shadow);
	if (ncwin->panel)
		del_panel(ncwin->panel);
	if (ncwin->spanel)
		del_panel(ncwin->spanel);
	if (ncwin->title)
		free(ncwin->title);
	if (parent)
		wrefresh(parent->panel->win);
	update_panels();
	doupdate();	
}
