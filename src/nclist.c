/**
 * File              : nclist.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 12.06.2023
 * Last Modified Date: 09.05.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#include "ncwidgets.h"
#include "struct.h"
#include "utils.h"
#include "keys.h"
#include "fm.h"

#include <curses.h>
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

void nc_list_refresh(NcWidget *ncwidget)
{
	NcList *nclist = (NcList*)ncwidget;

	int h, w, y, x;
	getmaxyx(nclist->ncwidget.ncwin.overlay, h, w);

	if (nclist->selected < 0)
		nclist->selected = 0;

	//scroll to selected
	while (nclist->selected > h-2 + nclist->ypos - 2 && 
			nclist->selected + 1 < nclist->size)
		nclist->ypos++;

	while (nclist->selected < nclist->ypos + 1 
			&& nclist->selected > 0)
		nclist->ypos--;

	// fill with blank 
	for (y = 0; y < h - 2; ++y)
		for (x = 0; x < w - 2; x++)
			if (y + nclist->ypos == nclist->selected 
					&& ncwidget->focused)
				mvwaddch(nclist->ncwidget.ncwin.overlay, y+1, x+1, ' '|A_REVERSE);
			else		
				mvwaddch(nclist->ncwidget.ncwin.overlay, y+1, x+1, ' ');
	
	//fill with data
	for (y = 0; y < h - 2 && y < nclist->size; ++y) {
		wmove(nclist->ncwidget.ncwin.overlay, y + 1, 1);		
		u8char_t *str = nclist->info[y + nclist->ypos]; 
		
		if (y + nclist->ypos == nclist->selected && 
				ncwidget->focused){
			// move chars for xpos
			str = &str[nclist->xpos];

			for (x = 0; x < w - 2 && str[x].utf8[0]; ++x) {
				if (str[x].utf8[0] == '\n') str[x].utf8[0] = ' ';
				if (str[x].utf8[0] == '\r') str[x].utf8[0] = ' ';
				wattron(nclist->ncwidget.ncwin.overlay, str[x].attr | A_REVERSE);
				waddstr(nclist->ncwidget.ncwin.overlay, str[x].utf8);	
				wattroff(nclist->ncwidget.ncwin.overlay, str[x].attr| A_REVERSE);
			}
		} else {
			for (x = 0; x < w - 2 && str[x].utf8[0]; ++x) {
				if (str[x].utf8[0] == '\n') str[x].utf8[0] = ' ';
				if (str[x].utf8[0] == '\r') str[x].utf8[0] = ' ';
				wattron(nclist->ncwidget.ncwin.overlay, str[x].attr);
				waddstr(nclist->ncwidget.ncwin.overlay, str[x].utf8);	
				wattroff(nclist->ncwidget.ncwin.overlay, str[x].attr);
			}
		}
	}

	wrefresh(nclist->ncwidget.ncwin.overlay);
}

void nc_list_set_value(NcList *nclist, char **value, int size)
{
	nclist->on_set_value(nclist, value, size);
}

void _nc_list_set_value(NcList *nclist, char **value, int size)
{
	nclist->info = malloc( 8 * size + 8);
	if (!nclist->info){
		return;
	}
	nclist->size = size;
	
	/* copy values */
	int i;
	for (i = 0; i < nclist->size; ++i) {
		nclist->info[i] = 
			str2ucharstr(value[i], nclist->ncwidget.ncwin.color);
	}

	nc_list_refresh((NcWidget*)nclist);
}

void nc_list_set_selected(NcList *nclist, int index){
	nclist->selected = index;
	nc_list_refresh((NcWidget*)nclist);
}

int nc_list_get_selected(NcList *nclist){
	return nclist->selected;
}

void nc_list_set_focused(NcWidget *ncwidget, bool focused)
{
	ncwidget->focused = focused;
	nc_win_activate(&ncwidget->ncwin);
	nc_list_refresh(ncwidget);
}

void nc_list_activate(
		NcWidget *ncwidget,
		void *userdata,
		NCRET (*callback)(NcWidget *, void *, chtype)		
		)
{
	NcList *nclist = (NcList*)ncwidget;

	nc_list_set_focused(ncwidget, true);

	chtype ch;
	while (ch != CTRL('x')) {
		ch = getch();
		// stop execution if callback not NULL
		if (callback){
			NCRET ret = callback(ncwidget, userdata, ch);
			if (ret == NCCONT)
				continue;
			else if (ret == NCSTOP)
				break;
		}

		//switch keys
		switch (ch) {
			case KEY_RIGHT:
				{
					u8char_t *str = nclist->info[nclist->selected];
					str = &str[nclist->xpos];
					int len = ucharstrlen(str); 
					int h, w;
					getmaxyx(nclist->ncwidget.ncwin.overlay, h, w);					
					if (len < w - 1){
						beep();
						break;
					}
					nclist->xpos++;
					nc_list_refresh(ncwidget);	
					break;
				}

			case KEY_LEFT:
				if (nclist->xpos - 1 < 0){
					beep();
					break;
				}				
				nclist->xpos--;
				nc_list_refresh(ncwidget);	
				break;				
			
			case KEY_DOWN:
				nclist->xpos = 0;
				if (nclist->selected + 1 >= nclist->size){
					beep();
					break;
				}
				nclist->selected++;
				nc_list_refresh(ncwidget);	
				break;

			case KEY_UP:
				nclist->xpos = 0;
				if (nclist->selected - 1 < 0){
					beep();
					break;
				}				
				nclist->selected--;
				nc_list_refresh(ncwidget);	
				break;				

			case KEY_NPAGE:
				{
					nclist->xpos = 0;
					int h, w;
					getmaxyx(nclist->ncwidget.ncwin.overlay, h, w);				
					int conent_h = h-2;
					nclist->selected += conent_h;
					if (nclist->selected >= nclist->size){
						nclist->selected = nclist->size - 1;
						nclist->ypos = nclist->size - conent_h;
					}
					nc_list_refresh(ncwidget);	
					break;				
				}

			case KEY_PPAGE:
				{
					nclist->xpos = 0;
					int h, w;
					getmaxyx(nclist->ncwidget.ncwin.overlay, h, w);				
					int conent_h = h-2;
					nclist->selected -= conent_h;
					if (nclist->selected < 0){
						nclist->selected = 0;
						nclist->ypos = 0;
					}
					nc_list_refresh(ncwidget);	
					break;				
				}				

			case KEY_MOUSE:
				{
					MEVENT event;
					if (getmouse(&event) == OK) {
						if (wenclose(nclist->ncwidget.ncwin.overlay, event.y, event.x)){
							int x, y, h, w, i;
							getbegyx(nclist->ncwidget.ncwin.overlay, y, x);
							getmaxyx(nclist->ncwidget.ncwin.overlay, h, w);
							if (event.bstate & BUTTON1_PRESSED){
								int selectedRow = event.y - y - 1;
								if (nclist->selected == selectedRow){
									if (callback){
										NCRET ret = callback(ncwidget, userdata, KEY_RETURN);
										if (ret == NCCONT)
											continue;
										else if (ret == NCSTOP)
											break;
									}
								}
								if (selectedRow + nclist->ypos < nclist->size)
									nclist->selected = selectedRow + nclist->ypos;	
								nc_list_refresh(ncwidget);
								break;
							} else if (event.bstate & MOUSE_SCROLL_UP){
								nclist->xpos = 0;
								if (nclist->selected - 1 < 0){
									/*beep();*/
									break;
								}				
								nclist->selected--;
								nc_list_refresh(ncwidget);	
								break;
							} else if (event.bstate & MOUSE_SCROLL_DOWN){
								nclist->xpos = 0;
								if (nclist->selected + 1 >= nclist->size){
									/*beep();*/
									break;
								}
								nclist->selected++;
								nc_list_refresh(ncwidget);	
								break;
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
	nc_widget_set_focused(ncwidget, false);
	nc_widget_refresh(ncwidget);
}

void nc_list_destroy(NcWidget *ncwidget)
{
	NcList *nclist = (NcList*)ncwidget;
	nc_win_destroy(&ncwidget->ncwin);
	int i;
	for (i = 0; i < nclist->size; ++i) {
		free(nclist->info[i]);
	}
	free(nclist->info);
	free(nclist);
}

NcWidget * nc_list_new(
		NcWin *parent,
		const char *title,
		int h, int w, int y, int x,
		int color,
		char **value,
		int size,
		bool box,
		bool shadow
		)
{
	NcList *nclist =  
		(NcList *)nc_win_new(parent, title, h, w, y, x, color, box, shadow);
	if (!nclist)
		return NULL;

	nclist = realloc(nclist, sizeof(NcList));
	if (!nclist)
		return NULL;
	
	nclist->ncwidget.type = NcWidgetTypeList;

	nclist->selected = 0;
	nclist->ypos     = 0;
	nclist->xpos     = 0;
	nclist->ncwidget.focused  = 0;

	nclist->ncwidget.on_refresh     = nc_list_refresh;
	nclist->ncwidget.on_set_focused = nc_list_set_focused;
	nclist->ncwidget.on_activate    = nc_list_activate;
	nclist->ncwidget.on_destroy	    = nc_list_destroy;

	nclist->on_set_value            = _nc_list_set_value;
	
	nc_list_set_value(nclist, value, size);

	return (NcWidget*)nclist;
}
