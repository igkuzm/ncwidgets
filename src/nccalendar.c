/**
 * File              : nccalendar.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 29.06.2023
 * Last Modified Date: 01.07.2023
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#include "nccalendar.h"
#include "keys.h"
#include "utils.h"
#include <ncurses.h>
#include <stdio.h>
#include <time.h>

const char * nc_calendar_dnames[] = 
{
	"  ",
	" 1", " 2", " 3", " 4", " 5", " 6", " 7", " 8", " 9", "10",
	"11", "12", "13", "14", "15", "16", "17", "18", "19", "20",
	"21", "22", "23", "24", "25", "26", "27", "28", "29", "30", 
	"31"
};

#define nc_calendar_iterate(it, tm) \
	it = *tm;\
	it.tm_mday = 1;\
	mktime(&it);\
	for (; it.tm_mon == tm->tm_mon; it.tm_mday++, mktime(&it))\

void
nc_calendar_refresh(
		nccalendar_t *nccalendar
		)
{
	attr_t attr = COLOR_PAIR(nccalendar->ncwin->color);

	// set date
	struct tm *tm = nccalendar->tm;
	tm->tm_hour = 0;
	tm->tm_min = 0;
	mktime(tm);
	
	int i, h, w, y, x;
	getmaxyx(nccalendar->ncwin->overlay, h, w);

	// fill with blank 
	for (y = 0; y < h - 2; ++y)
		for (x = 0; x < w - 2; x++)
			mvwaddch(nccalendar->ncwin->overlay, y+1, x+1, ' ');
	
	// print month name
	char mname[16];
	strftime(mname, 16, "%B", tm);
	int len = strchars(mname); 

	// get center of text (month name + year)
	len += 5;
	int start = w/2 - len/2;

	if (nccalendar->focused && nccalendar->selected == nccalendar_selected_month)
		wattron (nccalendar->ncwin->overlay, attr | A_REVERSE);
	mvwaddstr(nccalendar->ncwin->overlay, 1, start, mname);
	if (nccalendar->focused && nccalendar->selected == nccalendar_selected_month)
		wattroff(nccalendar->ncwin->overlay, attr | A_REVERSE);
	
	// print year
	waddch(nccalendar->ncwin->overlay, ' ');
	char year[5];
	sprintf(year, "%d", tm->tm_year + 1900);
	if (nccalendar->focused && nccalendar->selected == nccalendar_selected_year)
		wattron (nccalendar->ncwin->overlay, attr | A_REVERSE);
	waddstr(nccalendar->ncwin->overlay, year);
	if (nccalendar->focused && nccalendar->selected == nccalendar_selected_year)
		wattroff(nccalendar->ncwin->overlay, attr | A_REVERSE);
	 
	// print week names
	wmove(nccalendar->ncwin->overlay, 2, 1);		
	struct tm tw = *tm;
	for (i = 0; i < 7; i++){
		tw.tm_wday = nccalendar->startofweek + i;
		if (tw.tm_wday > 6)
			tw.tm_wday = 0;
		char wname[5];
		strftime(wname, 5, "%a", &tw);
		// terminate string after 2 char
		int index = uchar_index(wname, 2);
		wname[index] = 0;

		waddstr(nccalendar->ncwin->overlay, wname);
		if (i < 6)
			waddch(nccalendar->ncwin->overlay, ' ');
	}

	// fill with dates
	y = 3; 
	struct tm tp;
	nc_calendar_iterate(tp, tm){
		if (tp.tm_mday == 1){
			i = tp.tm_wday - nccalendar->startofweek;
			if (i < 0)
				i += 7;
		}
		x = i*3 + 1;
		if (nccalendar->focused 
				&& nccalendar->selected == nccalendar_selected_day 
				&& tp.tm_mday == tm->tm_mday)		
			wattron (nccalendar->ncwin->overlay, attr | A_REVERSE);
		mvwaddstr(nccalendar->ncwin->overlay, y, x, nc_calendar_dnames[tp.tm_mday]);
		if (nccalendar->focused 
				&& nccalendar->selected == nccalendar_selected_day 
				&& tp.tm_mday == tm->tm_mday)		
			wattroff(nccalendar->ncwin->overlay, attr | A_REVERSE);		
		if (++i == 7){
			i = 0;
			y++;	
		}
	}

	wrefresh(nccalendar->ncwin->overlay);
}

void nc_calendar_set(nccalendar_t *nccalendar, time_t time){
	nccalendar->tm = localtime(&time);
	nc_calendar_refresh(nccalendar);
}

nccalendar_t *
nc_calendar_new(
		PANEL *parent,
		const char *title,
		int y, int x,
		int color,
		time_t time,
		int startofweek,
		bool box,
		bool shadow
		)
{
	int h = 10, w = 22;
	nccalendar_t *nccalendar = malloc(sizeof(nccalendar_t));
	if (!nccalendar)
		return NULL;

	nccalendar->ncwin = nc_window_new(parent, title, h, w, y, x, color, box, shadow);
	if (!nccalendar->ncwin){
		free(nccalendar);
		return NULL;
	}

	nccalendar->tm = localtime(&time);

	nccalendar->focused  = 0;
	nccalendar->selected = 0;
	nccalendar->startofweek = startofweek;

	nc_calendar_set(nccalendar, time);

	return nccalendar;
}

void nc_calendar_set_focused(nccalendar_t *nccalendar, bool focused)
{
	nccalendar->focused  = focused;
	nc_calendar_refresh(nccalendar);
}

void nc_calendar_activate(
		nccalendar_t *nccalendar,
		void *userdata,
		CBRET (*callback)(void *userdata, enum SCREEN type, void *object, chtype key)		
		)
{

	nc_calendar_set_focused(nccalendar, true);

	chtype ch;
	while (ch != CTRL('x')) {
		ch = getch();
		// stop execution if callback not NULL
		if (callback){
			CBRET ret = callback(userdata, SCREEN_nccalendar, nccalendar, ch);
			if (ret == CBCONTINUE)
				continue;
			else if (ret == CBBREAK)
				break;
		}

		//switch keys
		switch (ch) {
			case KEY_TAB: case KEY_SPACE:
				{
					nccalendar->selected++;
					if (nccalendar->selected > nccalendar_selected_year)
						nccalendar->selected = nccalendar_selected_day; 
					nc_calendar_refresh(nccalendar);
					break;
				}
			case KEY_RIGHT: case '+':
				{
					if (nccalendar->selected == nccalendar_selected_day)
						nccalendar->tm->tm_mday++;
					else if (nccalendar->selected == nccalendar_selected_month)
						nccalendar->tm->tm_mon++;
					else if (nccalendar->selected == nccalendar_selected_year)
						nccalendar->tm->tm_year++;
					mktime(nccalendar->tm);
					nc_calendar_refresh(nccalendar);
					break;
				}

			case KEY_LEFT: case '-':
				{
					if (nccalendar->selected == nccalendar_selected_day)
						nccalendar->tm->tm_mday--;
					else if (nccalendar->selected == nccalendar_selected_month)
						nccalendar->tm->tm_mon--;
					else if (nccalendar->selected == nccalendar_selected_year)
						nccalendar->tm->tm_year--;
					mktime(nccalendar->tm);
					nc_calendar_refresh(nccalendar);
					break;
				}
			
			case KEY_DOWN:
				{
					if (nccalendar->selected == nccalendar_selected_day)
						nccalendar->tm->tm_mday += 7;
					else if (nccalendar->selected == nccalendar_selected_month)
						nccalendar->tm->tm_mon++;
					else if (nccalendar->selected == nccalendar_selected_year)
						nccalendar->tm->tm_year++;
					mktime(nccalendar->tm);
					nc_calendar_refresh(nccalendar);
					break;
				}

			case KEY_UP:
				{
					if (nccalendar->selected == nccalendar_selected_day)
						nccalendar->tm->tm_mday -= 7;
					else if (nccalendar->selected == nccalendar_selected_month)
						nccalendar->tm->tm_mon--;
					else if (nccalendar->selected == nccalendar_selected_year)
						nccalendar->tm->tm_year--;
					mktime(nccalendar->tm);
					nc_calendar_refresh(nccalendar);
					break;
				}

			case KEY_MOUSE:
				{
					MEVENT event;
					if (getmouse(&event) == OK) {
						if (wenclose(nccalendar->ncwin->overlay, event.y, event.x)){
							int x, y, h, w, i;
							getbegyx(nccalendar->ncwin->overlay, y, x);
							getmaxyx(nccalendar->ncwin->overlay, h, w);
							int selectedRow    = event.y - y;
							int selectedColumn = event.x - x;
							if (event.bstate & BUTTON1_PRESSED){
								if (selectedRow == 1) {
									if (selectedColumn < w/2){
										nccalendar->selected = nccalendar_selected_month;
										nc_calendar_refresh(nccalendar);
										break;
									}
									else if (selectedColumn > w/2){
										nccalendar->selected = nccalendar_selected_year;
										nc_calendar_refresh(nccalendar);
										break;
									}
								}
								else if (selectedRow > 2){
									nccalendar->selected = nccalendar_selected_day;
									nc_calendar_refresh(nccalendar);
									y = 3;
									struct tm tp;
									nc_calendar_iterate(tp, nccalendar->tm){
										if (tp.tm_mday == 1){
											i = tp.tm_wday - nccalendar->startofweek;
											if (i < 0)
												i += 7;
										}
										x = i*3 + 1;
										if (selectedRow == y 
												&& selectedColumn >= x
												&& selectedColumn < x + 3)
										{
											if (nccalendar->tm->tm_mday == tp.tm_mday){
												if (callback){
													CBRET ret = 
														callback(userdata, 
																SCREEN_nccalendar, 
																nccalendar, KEY_RETURN);
													if (ret == CBCONTINUE)
														continue;
													else if (ret == CBBREAK)
														break;
												}
											}
											*nccalendar->tm = tp;
											nc_calendar_refresh(nccalendar);
											break;
										}
										if (++i == 7){
											i = 0;
											y++;	
										}
									}

									break;									
								}
							} else if (event.bstate & MOUSE_SCROLL_UP){
								if (nccalendar->selected == nccalendar_selected_day)
									nccalendar->tm->tm_mday--;
								else if (nccalendar->selected == nccalendar_selected_month)
									nccalendar->tm->tm_mon--;
								else if (nccalendar->selected == nccalendar_selected_year)
									nccalendar->tm->tm_year--;
								mktime(nccalendar->tm);
								nc_calendar_refresh(nccalendar);
								break;
							} else if (event.bstate & MOUSE_SCROLL_DOWN){
								if (nccalendar->selected == nccalendar_selected_day)
									nccalendar->tm->tm_mday++;
								else if (nccalendar->selected == nccalendar_selected_month)
									nccalendar->tm->tm_mon++;
								else if (nccalendar->selected == nccalendar_selected_year)
									nccalendar->tm->tm_year++;
								mktime(nccalendar->tm);
								nc_calendar_refresh(nccalendar);
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
nc_calendar_destroy(
		nccalendar_t *nccalendar)
{
	nc_window_destroy(nccalendar->ncwin);
	free(nccalendar);
}
