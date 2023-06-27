/**
 * File              : ncentry.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 16.06.2023
 * Last Modified Date: 27.06.2023
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#include "ncentry.h"
#include "utils.h"
#include <ncurses.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

void
nc_enrty_refresh(ncentry_t *ncentry)
{
	int h, w, y, x;
	getmaxyx(ncentry->ncwin->overlay, h, w);

	u8char_t *str = ncentry->info; 

	//scroll to position
	ncentry->ypos     = 0;
	ncentry->xpos     = 0;	

	int width = w-2, height = h-2;
	if (ncentry->multiline){
		while (ncentry->position >= (height  + ncentry->ypos) * width)
			ncentry->ypos++;
	} else {
		while (ncentry->position >= width + ncentry->xpos)
			ncentry->xpos ++;
	}

	// fill with blank 
	for (y = 0; y < h-2; ++y)
		for (x = 0; x < w - 2; x++)
			mvwaddch(ncentry->ncwin->overlay, y+1, x+1, ' ');	

	// move string start positions
	size_t i = 0;
	i += ncentry->ypos * width;
	i += ncentry->xpos;
	
	// fill with data
	int lines = ncentry->multiline ? h-2 : 1; 
	bool has_position = false;
	for (y = 0; y < lines; ++y) {
		wmove(ncentry->ncwin->overlay, y + 1, 1);		
		
		for (x = 0; x < w - 2 && str[i].utf8[0]; ++x){
			if (str[i].utf8[0] == '\n'){
				i++;
				if (ncentry->multiline)
					break;
			}
			if (i == ncentry->position && ncentry->focused){
				wattron (ncentry->ncwin->overlay, str[i].attr | A_REVERSE);
				waddstr (ncentry->ncwin->overlay, str[i].utf8);
				wattroff(ncentry->ncwin->overlay, str[i].attr | A_REVERSE);
				has_position = true;
			} else
				waddstr (ncentry->ncwin->overlay, str[i].utf8);
			i++;
		}
		
		if (i == ncentry->position && ncentry->focused && !has_position && x < w - 2){
			wattron (ncentry->ncwin->overlay, A_REVERSE);
			waddch  (ncentry->ncwin->overlay, ' ');
			wattroff(ncentry->ncwin->overlay, A_REVERSE);
			has_position = true;
		}
	}

	wrefresh(ncentry->ncwin->overlay);
}

void
nc_entry_set_value(
		ncentry_t *ncentry,
		const char *value
		)
{
	ncentry->info = str2ucharstr(value, ncentry->ncwin->color);
	nc_enrty_refresh(ncentry);
}

ncentry_t *
nc_entry_new(
		PANEL *parent,
		const char *title,
		int h, int w, int y, int x,
		int color,
		const char *value,
		bool multiline,
		bool box,
		bool shadow
		)
{
	ncentry_t *ncentry = malloc(sizeof(ncentry_t));
	if (!ncentry)
		return NULL;

	ncentry->ncwin = nc_window_new(parent, title, h, w, y, x, color, box, shadow);
	if (!ncentry->ncwin){
		free(ncentry);
		return NULL;
	}

	ncentry->multiline= multiline;
	ncentry->position = 0;
	ncentry->ypos     = 0;
	ncentry->xpos     = 0;
	ncentry->focused  = 0;

	if (value && strlen(value)){
		ncentry->allocated = strlen(value) * sizeof(u8char_t);
		nc_entry_set_value(ncentry, value);
		//set position
		ncentry->position = ucharstrlen(ncentry->info);
	} else {
		ncentry->allocated = BUFSIZ;
		ncentry->info = malloc(BUFSIZ);
		if (!ncentry->info)
			return NULL;
		ncentry->info[0].utf8[0] = 0;
	} 


	return ncentry;
}

void nc_entry_destroy(ncentry_t *ncentry)
{
	nc_window_destroy(ncentry->ncwin);
	free(ncentry->info);
	free(ncentry);
}

void nc_entry_set_position(ncentry_t *ncentry, size_t position){
	ncentry->position = position;
	nc_enrty_refresh(ncentry);
}

size_t nc_entry_get_position(ncentry_t *ncentry){
	return ncentry->position;
}

void nc_entry_set_focused(ncentry_t *ncentry, bool focused){
	ncentry->focused = focused;
	nc_enrty_refresh(ncentry);
}

void nc_entry_add_char(ncentry_t *ncentry, u8char_t ch)
{
	// create new buffer
	size_t len = ucharstrlen(ncentry->info);
	u8char_t *buf;
	if (len * sizeof(u8char_t) >= ncentry->allocated)
		buf = malloc(ncentry->allocated + BUFSIZ);
	else
		buf = malloc(ncentry->allocated + BUFSIZ);
	if (!buf)
		return;
	ncentry->allocated += BUFSIZ;

	//put char
	size_t i, k;
	for (i = 0, k = 0; i < len; i++, k++) {
		if (i == ncentry->position){
			buf[k++] = ch;
		}
		buf[k] = ncentry->info[i];
	}
	
	if (i == ncentry->position)
		buf[k++] = ch;
	
	buf[k].utf8[0] = 0;

	free(ncentry->info);
	ncentry->info = buf;
	ncentry->position++;
}

void nc_entry_remove_char(ncentry_t *ncentry)
{
	// create new buffer
	size_t len = ucharstrlen(ncentry->info);
	u8char_t *buf = malloc(ncentry->allocated);

	size_t i, k;
	for (i = 0, k = 0; i < len; i++, k++) {
		if (i == ncentry->position - 1)
			i++;
		buf[k] = ncentry->info[i];
	}
	buf[k].utf8[0] = 0;

	free(ncentry->info);
	ncentry->info = buf;
	ncentry->position--;
}


void nc_entry_activate(
		ncentry_t *ncentry,
		void *userdata,
		CBRET (*callback)(void *userdata, enum SCREEN type, void *object, chtype key)
		)
{
	nc_entry_set_focused(ncentry, true);

	chtype ch;
	while (ch != CTRL('x')) {
		ch = getch();
		// stop execution if callback not NULL
		if (callback){
			CBRET ret = callback(userdata, SCREEN_ncentry, ncentry, ch);
			if (ret == CBBREAK)
				break;
			else if (ret == CBCONTINUE)
				continue;
		}

		//switch keys
		switch (ch) {
			case KEY_RIGHT:
				{
					size_t len = ucharstrlen(ncentry->info);
					if (ncentry->position < len){
						ncentry->position++;
						nc_enrty_refresh(ncentry);
					} else {
						beep();
					}
					break;
				}	

			case KEY_END:
				{
					int h, w;
					getmaxyx(ncentry->ncwin->overlay, h, w);
					int width = w-2;
					size_t len = ucharstrlen(ncentry->info);
					if (ncentry->position + width < len)
						ncentry->position += width - 1;
					else
						ncentry->position = len;
					nc_enrty_refresh(ncentry);
					break;
				}	

			case KEY_LEFT:
				{
					if (ncentry->position > 0){
						ncentry->position--;
						nc_enrty_refresh(ncentry);
					}
					else{
						beep();
					} 
					break;
				}
			
			case KEY_HOME:
				{
					int h, w;
					getmaxyx(ncentry->ncwin->overlay, h, w);
					int width = w-2;
					if (ncentry->position > width)
						ncentry->position -= width;
					else 
						ncentry->position = 0;

					nc_enrty_refresh(ncentry);
					break;
				}	
				
			case KEY_DOWN:
				{
					if (!ncentry->multiline)
						break;
					int h, w;
					getmaxyx(ncentry->ncwin->overlay, h, w);
					int width = w-2;
					size_t len = ucharstrlen(ncentry->info);
					if (ncentry->position + width < len)
						ncentry->position += width;
					else
						ncentry->position = len;
					nc_enrty_refresh(ncentry);
					break;
				}
			case KEY_NPAGE:
				{
					if (!ncentry->multiline)
						break;					
					int h, w;
					getmaxyx(ncentry->ncwin->overlay, h, w);
					int width = w-2, height = h - 2;					
					size_t len = ucharstrlen(ncentry->info);
					if (ncentry->position + width * height < len)
						ncentry->position += width * height;
					else
						ncentry->position = len;
					nc_enrty_refresh(ncentry);
					break;
				}
			
			case KEY_UP:
				{
					if (!ncentry->multiline)
						break;
					int h, w;
					getmaxyx(ncentry->ncwin->overlay, h, w);
					int width = w-2;
					if (ncentry->position > width)
						ncentry->position -= width;
					else
						ncentry->position = 0;
					nc_enrty_refresh(ncentry);
					break;
				}				
			
			case KEY_PPAGE:
				{
					if (!ncentry->multiline)
						break;
					int h, w;
					getmaxyx(ncentry->ncwin->overlay, h, w);
					int width = w-2, height = h - 2;					
					if (ncentry->position > width * height)
						ncentry->position -= width * height;
					else
						ncentry->position = 0;
					nc_enrty_refresh(ncentry);
					break;
				}

			case KEY_BACKSPACE: case KEY_DELETE:
				if (ncentry->position == 0)
					break;
				nc_entry_remove_char(ncentry);
				nc_enrty_refresh(ncentry);				
				break;
			
			case KEY_TAB:
				{
					u8char_t u8ch;
					u8ch.utf8[0] = '\t'; 
					u8ch.utf8[1] = 0; 
					nc_entry_add_char(ncentry, u8ch);
					nc_enrty_refresh(ncentry);
				}

			case KEY_ENTER: case KEY_RETURN: case '\r':
				{
					u8char_t u8ch;
					u8ch.utf8[0] = '\n'; 
					u8ch.utf8[1] = 0; 
					nc_entry_add_char(ncentry, u8ch);
					nc_enrty_refresh(ncentry);
				}				

			case KEY_MOUSE:
				{
					MEVENT event;
					if (getmouse(&event) == OK) {
						if (wenclose(ncentry->ncwin->overlay, event.y, event.x)){
							int x, y, h, w, i;
							getbegyx(ncentry->ncwin->overlay, y, x);
							getmaxyx(ncentry->ncwin->overlay, h, w);
							int width = w-2, height = h - 2;					
							size_t len = ucharstrlen(ncentry->info);
							if (event.bstate & BUTTON1_PRESSED){
								int selectedRow    = event.y - y - 1;
								int selectedColumn = event.x - x - 1;

								ncentry->position = (ncentry->ypos + selectedRow) * width;
								ncentry->position += ncentry->xpos + selectedColumn;

								if (ncentry->position > len)
									ncentry->position = len;

								nc_enrty_refresh(ncentry);
								break;
							} else if (event.bstate & MOUSE_SCROLL_UP){
								if (!ncentry->multiline)
									break;
								if (ncentry->position > width)
									ncentry->position -= width;
								else
									ncentry->position = 0;
								nc_enrty_refresh(ncentry);
								break;
							} else if (event.bstate & MOUSE_SCROLL_DOWN){
								if (!ncentry->multiline)
									break;
								int h, w;
								getmaxyx(ncentry->ncwin->overlay, h, w);
								int width = w-2;
								size_t len = ucharstrlen(ncentry->info);
								if (ncentry->position + width < len)
									ncentry->position += width;
								else
									ncentry->position = len;
								nc_enrty_refresh(ncentry);
								break;
							}
						}
					}
					break;
				}				
			
			default:
				{
					unsigned char c = (char)ch;
					if (c < 32){
						beep();
						break;
					}
					u8char_t u8ch;
					u8ch.attr = COLOR_PAIR(ncentry->ncwin->color);
					if (c >= 252){/* 6-bytes */
						u8ch.utf8[0] = c; 
						u8ch.utf8[1] = getch(); 
						u8ch.utf8[2] = getch(); 
						u8ch.utf8[3] = getch(); 
						u8ch.utf8[4] = getch(); 
						u8ch.utf8[5] = getch(); 
						u8ch.utf8[6] = 0; 
					} 
					else if (c >= 248){/* 5-bytes */
						u8ch.utf8[0] = c; 
						u8ch.utf8[1] = getch(); 
						u8ch.utf8[2] = getch(); 
						u8ch.utf8[3] = getch(); 
						u8ch.utf8[4] = getch(); 
						u8ch.utf8[5] = 0; 
					}
					else if (c >= 240){/* 4-bytes */
						u8ch.utf8[0] = c; 
						u8ch.utf8[1] = getch(); 
						u8ch.utf8[2] = getch(); 
						u8ch.utf8[3] = getch(); 
						u8ch.utf8[4] = 0; 
					} 
					else if (c >= 224){/* 3-bytes */
						u8ch.utf8[0] = c; 
						u8ch.utf8[1] = getch(); 
						u8ch.utf8[2] = getch(); 
						u8ch.utf8[3] = 0; 
					}
					else if (c >= 192){/* 2-bytes */
						u8ch.utf8[0] = c; 
						u8ch.utf8[1] = getch(); 
						u8ch.utf8[2] = 0; 
					} 
					else{/* 1-byte */
						u8ch.utf8[0] = c; 
						u8ch.utf8[1] = 0; 
					} 
					nc_entry_add_char(ncentry, u8ch);
					nc_enrty_refresh(ncentry);
				}
		}
	}
}


