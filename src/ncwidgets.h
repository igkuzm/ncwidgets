/**
 * File              : ncwidgets.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 12.06.2023
 * Last Modified Date: 09.05.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#ifndef NC_WIDGETS_H
#define NC_WIDGETS_H

#include <curses.h>
#include <panel.h>
#include "utils.h"
#include "fm.h"

#include "colors.h"

/* init curses */
void nc_init(const char *locale, int color);

/* quit curses */
void nc_quit();

/* return codes of callback */
typedef enum NCRET {
	NCNONE, // do nothing
	NCCONT, // continue with input char
	NCSTOP  // stop function

} NCRET;

typedef enum NcWidgetType {
	NcWidgetTypeButton,
	NcWidgetTypeLabel,
	NcWidgetTypeCalendar,
	NcWidgetTypeEntry,
	NcWidgetTypeList,
	NcWidgetTypeSelection,
	NcWidgetTypeFselect,
} NcWidgetType;

/* NcWin is curses window with panel, box, shadow and 
 * title. All widgets are Ncwin. You may put windgets into
 * main curses screen or into NcWin as parent */
typedef struct NcWin NcWin;
NcWin * nc_win_new(
		NcWin *parent,
		const char *title,
		int h, int w, int y, int x,
		int color,
		bool box,
		bool shadow
		);

int nc_win_move(NcWin *ncwin, int y, int x);
int nc_win_resize(NcWin *ncwin, int h, int w);
void nc_win_set_title(NcWin *ncwin, const char *title);
int nc_win_hide(NcWin *ncwin);
int nc_win_show(NcWin *ncwin);
bool nc_win_hidden(NcWin *ncwin);
int nc_win_activate(NcWin *ncwin);
void nc_win_destroy(NcWin *ncwin);

/* widget is NcWin with controls */
typedef struct NcWidget NcWidget;
void nc_widget_refresh(NcWidget *widget);
void nc_widget_set_focused(NcWidget *widget, bool focused);
void nc_widget_activate(
		NcWidget *widget, 
		void *userdata,
		NCRET callback(NcWidget *widget, void *userdata, chtype ch)
		);
void nc_widget_destroy(NcWidget *widget);
int nc_widget_move(NcWidget *widget, int y, int x);
int nc_widget_resize(NcWidget *widget, int h, int w);

/* label - NcWindget with text */
typedef struct NcLabel NcLabel;
NcWidget * nc_label_new(
		NcWin *parent,
		int y, int x,
		int color,
		const char *text,
		bool box,
		bool shadow
		);

/* button - NcLable with click callback */
typedef NcLabel NcButton;
NcWidget *nc_button_new(
		NcWin *parent,
		int h, int w, int y, int x,
		int color,
		const char *text,
		bool box,
		bool shadow
		);

/* calendar widget */
typedef struct NcCalendar NcCalendar;
NcWidget * nc_calendar_new(
		NcWin *parent,
		const char *title,
		int y, int x,
		int color,
		time_t time,
		int startofweek,
		bool box,
		bool shadow
		);

void   nc_calendar_set(NcCalendar *nccalendar, time_t time);
time_t nc_calendar_get(NcCalendar *nccalendar);

typedef struct NcEntry NcEntry;
NcWidget * nc_entry_new(
		NcWin *parent,
		const char *title,
		int h, int w, int y, int x,
		int color,
		const char *value,
		bool multiline,
		bool box,
		bool shadow
		);

void nc_entry_set_value(NcEntry *ncentry, const char *value);
char *nc_entry_get_value(NcEntry *ncentry);
void nc_entry_set_position(NcEntry *ncentry, size_t position);
size_t nc_entry_get_position(NcEntry *ncentry);


/* file selection */
typedef struct NcFselect NcFselect;
NcWidget *nc_fselect_new(
		NcWin *parent, 
		int h, int w, int y, int x, 
		int color, 
		const char *path,
		attr_t file_attr,
		attr_t dir_attr,
		attr_t link_attr,
		attr_t other_attr,		
		bool box, 
		bool shadow
		);

void nc_fselect_set(NcFselect *fselect, const char *path);
char * nc_fselect_get(NcFselect *fselect);

/* list/menu widget */
typedef struct NcList NcList;
NcWidget * nc_list_new(
		NcWin *parent,
		const char *title,
		int h, int w, int y, int x,
		int color,
		char **value,
		int size,
		bool box,
		bool shadow
		);

void nc_list_set_value(NcList *nclist, char **value, int size);
void nc_list_set_selected(NcList *nclist, int index);
int nc_list_get_selected(NcList *nclist);

/* selection list */
typedef struct NcSelection NcSelection;
NcWidget *nc_selection_new(
		NcWin *parent, 
		const char *title, 
		int h, int w, int y, int x, 
		int color, 
		char **selections,
		int count,
		int * selected,
		bool multiselect,
		char **value, 
		int size, 
		bool box, 
		bool shadow
		);

void nc_selection_set(
		NcSelection *s, 
		char **value, 
		int size, 
		char **selections, 
		int count,
		int * selected
		);

void nc_selection_select(NcSelection *s, int index, int selected);
int *nc_selection_get(NcSelection *s);


/* group widgets to traverse */
typedef struct NcGroup NcGroup;
NcGroup *nc_group_new();
NcGroup *nc_group_free(NcGroup *group);
void nc_group_add(NcGroup *group, NcWidget *widget);
void nc_group_remove(NcGroup *group, NcWidget *widget);
void nc_group_activate(
		NcGroup *group, 
		NcWidget *selected,
		void *userdata,
		NCRET callback(NcWidget *widget, void *userdata, chtype ch)
		);

int nc_dialog(
		int color,
		const char *title,
		const char *msg, 
		const char *buttons[], int count);

#endif /* ifndef NC_WIDGETS_H */
