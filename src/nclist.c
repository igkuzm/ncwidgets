/**
 * File              : nclist.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 12.06.2023
 * Last Modified Date: 26.06.2023
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#include "nclist.h"
#include "ncwin.h"
#include "utils.h"
#include "keys.h"
#include "types.h"

#include <curses.h>
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

void
nc_list_refresh(
		nclist_t *nclist
		)
{
	int h, w, y, x;
	getmaxyx(nclist->ncwin->overlay, h, w);

	if (nclist->selected < 0)
		nclist->selected = 0;

	//scroll to selected
	while (nclist->selected > h-2 + nclist->ypos - 2 && nclist->selected + 1 < nclist->size)
		nclist->ypos++;

	while (nclist->selected < nclist->ypos + 1 && nclist->selected > 0)
		nclist->ypos--;

	// fill with blank 
	for (y = 0; y < h - 2; ++y)
		for (x = 0; x < w - 2; x++)
			if (y + nclist->ypos == nclist->selected && nclist->focused)
				mvwaddch(nclist->ncwin->overlay, y+1, x+1, ' '|A_REVERSE);
			else		
				mvwaddch(nclist->ncwin->overlay, y+1, x+1, ' ');
	
	//fill with data
	for (y = 0; y < h - 2 && y < nclist->size; ++y) {
		wmove(nclist->ncwin->overlay, y + 1, 1);		
		u8char_t *str = nclist->info[y + nclist->ypos]; 
		
		if (y + nclist->ypos == nclist->selected && nclist->focused){
			// move chars for xpos
			str = &str[nclist->xpos];

			for (x = 0; x < w - 2 && str[x].utf8[0]; ++x) {
				if (str[x].utf8[0] == '\n') str[x].utf8[0] = ' ';
				if (str[x].utf8[0] == '\r') str[x].utf8[0] = ' ';
				wattron(nclist->ncwin->overlay, str[x].attr | A_REVERSE);
				waddstr(nclist->ncwin->overlay, str[x].utf8);	
				wattroff(nclist->ncwin->overlay, str[x].attr| A_REVERSE);
			}
		} else {
			for (x = 0; x < w - 2 && str[x].utf8[0]; ++x) {
				if (str[x].utf8[0] == '\n') str[x].utf8[0] = ' ';
				if (str[x].utf8[0] == '\r') str[x].utf8[0] = ' ';
				wattron(nclist->ncwin->overlay, str[x].attr);
				waddstr(nclist->ncwin->overlay, str[x].utf8);	
				wattroff(nclist->ncwin->overlay, str[x].attr);
			}
		}
	}

	wrefresh(nclist->ncwin->overlay);
}

void
nc_list_set_value(
		nclist_t *nclist,
		const char **value,
		int size		
		)
{
	nclist->info = malloc( 8 * size + 8);
	if (!nclist->info){
		return;
	}
	nclist->size = size;
	
	/* copy values */
	int i;
	for (i = 0; i < nclist->size; ++i) {
		nclist->info[i] = str2ucharstr(value[i], nclist->ncwin->color);
	}

	nc_list_refresh(nclist);
}

nclist_t *
nc_list_new(
		PANEL *parent,
		const char *title,
		int h, int w, int y, int x,
		int color,
		const char **value,
		int size,
		bool box,
		bool shadow
		)
{
	nclist_t *nclist = malloc(sizeof(nclist_t));
	if (!nclist)
		return NULL;

	nclist->ncwin = nc_window_new(parent, title, h, w, y, x, color, box, shadow);
	if (!nclist->ncwin){
		free(nclist);
		return NULL;
	}

	nclist->selected = 0;
	nclist->ypos     = 0;
	nclist->xpos     = 0;
	nclist->focused  = 0;

	nc_list_set_value(nclist, value, size);

	return nclist;
}

void nc_list_set_selected(nclist_t *nclist, int index){
	nclist->selected = index;
	nc_list_refresh(nclist);
}

int nc_list_get_selected(nclist_t *nclist){
	return nclist->selected;
}

void nc_list_set_focused(nclist_t *nclist, bool focused)
{
	nclist->focused = focused;
	nc_list_refresh(nclist);
}

void nc_list_activate(
		nclist_t *nclist,
		void *userdata,
		CBRET (*callback)(void *userdata, enum SCREEN type, void *object, chtype key)		
		)
{
	nc_list_set_focused(nclist, true);

	chtype ch;
	while (ch != CTRL('x')) {
		ch = getch();
		// stop execution if callback not NULL
		if (callback){
			CBRET ret = callback(userdata, SCREEN_nclist, nclist, ch);
			if (ret == CBCONTUNUE)
				continue;
			else if (ret == CBBREAK)
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
					getmaxyx(nclist->ncwin->overlay, h, w);					
					if (len < w - 1){
						beep();
						break;
					}
					nclist->xpos++;
					nc_list_refresh(nclist);	
					break;
				}

			case KEY_LEFT:
				if (nclist->xpos - 1 < 0){
					beep();
					break;
				}				
				nclist->xpos--;
				nc_list_refresh(nclist);	
				break;				
			
			case KEY_DOWN:
				nclist->xpos = 0;
				if (nclist->selected + 1 >= nclist->size){
					beep();
					break;
				}
				nclist->selected++;
				nc_list_refresh(nclist);	
				break;

			case KEY_UP:
				nclist->xpos = 0;
				if (nclist->selected - 1 < 0){
					beep();
					break;
				}				
				nclist->selected--;
				nc_list_refresh(nclist);	
				break;				

			case KEY_NPAGE:
				{
					nclist->xpos = 0;
					int h, w;
					getmaxyx(nclist->ncwin->overlay, h, w);				
					int conent_h = h-2;
					nclist->selected += conent_h;
					if (nclist->selected >= nclist->size){
						nclist->selected = nclist->size - 1;
						nclist->ypos = nclist->size - conent_h;
					}
					nc_list_refresh(nclist);	
					break;				
				}

			case KEY_PPAGE:
				{
					nclist->xpos = 0;
					int h, w;
					getmaxyx(nclist->ncwin->overlay, h, w);				
					int conent_h = h-2;
					nclist->selected -= conent_h;
					if (nclist->selected < 0){
						nclist->selected = 0;
						nclist->ypos = 0;
					}
					nc_list_refresh(nclist);	
					break;				
				}				

			case KEY_MOUSE:
				{
					MEVENT event;
					if (getmouse(&event) == OK) {
						if (wenclose(nclist->ncwin->overlay, event.y, event.x)){
							int x, y, h, w, i;
							getbegyx(nclist->ncwin->overlay, y, x);
							getmaxyx(nclist->ncwin->overlay, h, w);
							if (event.bstate & BUTTON1_PRESSED){
								int selectedRow = event.y - y - 1;
								if (nclist->selected == selectedRow){
									ch = KEY_RETURN;
									break;
								}
								if (selectedRow + nclist->ypos < nclist->size)
									nclist->selected = selectedRow + nclist->ypos;	
								nc_list_refresh(nclist);
								break;
							} else if (event.bstate & MOUSE_SCROLL_UP){
								nclist->xpos = 0;
								if (nclist->selected - 1 < 0){
									/*beep();*/
									break;
								}				
								nclist->selected--;
								nc_list_refresh(nclist);	
								break;
							} else if (event.bstate & MOUSE_SCROLL_DOWN){
								nclist->xpos = 0;
								if (nclist->selected + 1 >= nclist->size){
									/*beep();*/
									break;
								}
								nclist->selected++;
								nc_list_refresh(nclist);	
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
}

void
nc_list_destroy(
		nclist_t *nclist)
{
	nc_window_destroy(nclist->ncwin);
	int i;
	for (i = 0; i < nclist->size; ++i) {
		free(nclist->info[i]);
	}
	free(nclist->info);
	free(nclist);
}

