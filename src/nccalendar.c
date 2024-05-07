/**
 * File              : nccalendar.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 29.06.2023
 * Last Modified Date: 08.05.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#include "ncwidgets.h"
#include "struct.h"
#include "keys.h"
#include "utils.h"
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
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

void nc_calendar_refresh(NcWidget *ncwidget)
{
	NcCalendar *nccalendar = (NcCalendar *)ncwidget;
	attr_t attr = COLOR_PAIR(nccalendar->ncwidget.ncwin.color);

	// set date
	struct tm *tm = nccalendar->tm;
	tm->tm_hour = 0;
	tm->tm_min = 0;
	mktime(tm);
	
	int i, h, w, y, x;
	getmaxyx(nccalendar->ncwidget.ncwin.overlay, h, w);

	// fill with blank 
	for (y = 0; y < h - 2; ++y)
		for (x = 0; x < w - 2; x++)
			mvwaddch(nccalendar->ncwidget.ncwin.overlay, y+1, x+1, ' ');
	
	// print month name
	char mname[16];
	strftime(mname, 16, "%B", tm);
	int len = strchars(mname); 

	// get center of text (month name + year)
	len += 5;
	int start = w/2 - len/2;

	if (ncwidget->focused && nccalendar->selected == nccalendar_selected_month)
		wattron (nccalendar->ncwidget.ncwin.overlay, attr | A_REVERSE);
	mvwaddstr(nccalendar->ncwidget.ncwin.overlay, 1, start, mname);
	if (ncwidget->focused && nccalendar->selected == nccalendar_selected_month)
		wattroff(nccalendar->ncwidget.ncwin.overlay, attr | A_REVERSE);
	
	// print year
	waddch(nccalendar->ncwidget.ncwin.overlay, ' ');
	char year[5];
	sprintf(year, "%d", tm->tm_year + 1900);
	if (ncwidget->focused && nccalendar->selected == nccalendar_selected_year)
		wattron (nccalendar->ncwidget.ncwin.overlay, attr | A_REVERSE);
	waddstr(nccalendar->ncwidget.ncwin.overlay, year);
	if (ncwidget->focused && nccalendar->selected == nccalendar_selected_year)
		wattroff(nccalendar->ncwidget.ncwin.overlay, attr | A_REVERSE);
	 
	// print week names
	wmove(nccalendar->ncwidget.ncwin.overlay, 2, 1);		
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

		waddstr(nccalendar->ncwidget.ncwin.overlay, wname);
		if (i < 6)
			waddch(nccalendar->ncwidget.ncwin.overlay, ' ');
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
		if (ncwidget->focused 
				&& nccalendar->selected == nccalendar_selected_day 
				&& tp.tm_mday == tm->tm_mday)		
			wattron (nccalendar->ncwidget.ncwin.overlay, attr | A_REVERSE);
		mvwaddstr(nccalendar->ncwidget.ncwin.overlay, y, x, nc_calendar_dnames[tp.tm_mday]);
		if (ncwidget->focused 
				&& nccalendar->selected == nccalendar_selected_day 
				&& tp.tm_mday == tm->tm_mday)		
			wattroff(nccalendar->ncwidget.ncwin.overlay, attr | A_REVERSE);		
		if (++i == 7){
			i = 0;
			y++;	
		}
	}

	wrefresh(nccalendar->ncwidget.ncwin.overlay);
}

void nc_calendar_set_focused(NcWidget *ncwidget, bool focused)
{
	ncwidget->focused  = focused;
	nc_win_activate(&ncwidget->ncwin);
	nc_calendar_refresh(ncwidget);
}

void nc_calendar_set(NcCalendar *nccalendar, time_t time){
	nccalendar->tm = localtime(&time);
	nc_calendar_refresh((NcWidget*)nccalendar);
}

time_t nc_calendar_get(NcCalendar *nccalendar){
	return mktime(nccalendar->tm);
}

void nc_calendar_activate(
		NcWidget *ncwidget,
		void *userdata,
		NCRET (*callback)(NcWidget *, void *, chtype)		
		)
{
	NcCalendar *nccalendar = (NcCalendar *)ncwidget;

	nc_calendar_set_focused(ncwidget, true);

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
			case KEY_TAB: case KEY_SPACE:
				{
					nccalendar->selected++;
					if (nccalendar->selected > nccalendar_selected_year)
						nccalendar->selected = nccalendar_selected_day; 
					nc_calendar_refresh(ncwidget);
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
					nc_calendar_refresh(ncwidget);
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
					nc_calendar_refresh(ncwidget);
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
					nc_calendar_refresh(ncwidget);
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
					nc_calendar_refresh(ncwidget);
					break;
				}

			case KEY_MOUSE:
				{
					MEVENT event;
					if (getmouse(&event) == OK) {
						if (wenclose(nccalendar->ncwidget.ncwin.overlay, event.y, event.x)){
							int x, y, h, w, i;
							getbegyx(nccalendar->ncwidget.ncwin.overlay, y, x);
							getmaxyx(nccalendar->ncwidget.ncwin.overlay, h, w);
							int selectedRow    = event.y - y;
							int selectedColumn = event.x - x;
							if (event.bstate & BUTTON1_PRESSED){
								if (selectedRow == 1) {
									if (selectedColumn < w/2){
										nccalendar->selected = nccalendar_selected_month;
										nc_calendar_refresh(ncwidget);
										break;
									}
									else if (selectedColumn > w/2){
										nccalendar->selected = nccalendar_selected_year;
										nc_calendar_refresh(ncwidget);
										break;
									}
								}
								else if (selectedRow > 2){
									nccalendar->selected = nccalendar_selected_day;
									nc_calendar_refresh(ncwidget);
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
													NCRET ret = 
														callback(ncwidget, userdata, KEY_RETURN);
													if (ret == NCCONT)
														continue;
													else if (ret == NCSTOP)
														break;
												}
											}
											*nccalendar->tm = tp;
											nc_calendar_refresh(ncwidget);
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
								nc_calendar_refresh(ncwidget);
								break;
							} else if (event.bstate & MOUSE_SCROLL_DOWN){
								if (nccalendar->selected == nccalendar_selected_day)
									nccalendar->tm->tm_mday++;
								else if (nccalendar->selected == nccalendar_selected_month)
									nccalendar->tm->tm_mon++;
								else if (nccalendar->selected == nccalendar_selected_year)
									nccalendar->tm->tm_year++;
								mktime(nccalendar->tm);
								nc_calendar_refresh(ncwidget);
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


void nc_calendar_destroy(NcWidget *ncwidget)
{
	nc_win_destroy(&ncwidget->ncwin);
	free(ncwidget);
}

NcWidget *
nc_calendar_new(
		NcWin *parent,
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
	NcCalendar *nccalendar  = 
		(NcCalendar *)nc_win_new(parent, title, h, w, y, x, color, box, shadow);
	if (!nccalendar)
		return NULL;

	nccalendar = realloc(nccalendar, sizeof(NcCalendar));
	if (!nccalendar)
		return NULL;
	
	nccalendar->ncwidget.type = NcWidgetTypeCalendar;

	nccalendar->tm = localtime(&time);

	nccalendar->ncwidget.focused  = 0;
	nccalendar->selected = 0;
	nccalendar->startofweek = startofweek;

	nc_calendar_set(nccalendar, time);
	nccalendar->ncwidget.on_refresh     = nc_calendar_refresh;
	nccalendar->ncwidget.on_set_focused = nc_calendar_set_focused;
	nccalendar->ncwidget.on_activate    = nc_calendar_activate;
	nccalendar->ncwidget.on_destroy	    = nc_calendar_destroy;

	return (NcWidget*)nccalendar;
}
