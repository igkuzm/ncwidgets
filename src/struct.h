/**
 * File              : struct.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 08.05.2024
 * Last Modified Date: 08.05.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */
#ifndef NCWIDGETS_STRUCTURES_H
#define NCWIDGETS_STRUCTURES_H

#include "ncwidgets.h"

/* structs */
struct NcWin {
	struct NcWin *parent;
	PANEL *spanel;
	PANEL *panel;
	WINDOW *overlay;
	WINDOW *shadow;
	int color;
	u8char_t *title;
	bool box;
};

struct NcWidget {
	NcWin ncwin;
	NcWidgetType type;
	bool focused;
	void (*on_refresh)(NcWidget *widget);
	void (*on_set_focused)(NcWidget *widget, bool focused);
	void (*on_activate)(
		NcWidget *widget, 
		void *userdata,
		NCRET callback(NcWidget *widget, void *userdata, chtype ch)
		);
	void (*on_destroy)(NcWidget *widget);
	int key;
	void *userdata;
};

enum nccalendar_selected{
	nccalendar_selected_day,
	nccalendar_selected_month,
	nccalendar_selected_year,
};

struct NcCalendar {
	NcWidget ncwidget;
	struct tm *tm;
	int startofweek;
	enum nccalendar_selected selected;
};

struct NcEntry {
	NcWidget ncwidget;
	u8char_t *info;
	size_t allocated;
	bool multiline;
	size_t position;
	int ypos;	
	int xpos;	
};

struct NcLabel {
	NcWidget ncwidget;
	u8char_t **info;
	int lines;
};

struct NcList {
	NcWidget ncwidget;
	u8char_t **info;
	int size;
	int selected;
	int ypos;	
	int xpos;	
	void (*on_set_value)(NcList *nclist, char **value, int size);
};

void nc_list_activate(
		NcWidget *ncwidget,
		void *userdata,
		NCRET (*callback)(NcWidget *, void *, chtype)		
		);

struct NcFselect {
	NcList nclist;
	char path[BUFSIZ];
	bool focused;
	attr_t file_attr;
	attr_t dir_attr;
	attr_t link_attr;
	attr_t other_attr;
	NCRET (*callback)(NcWidget *, void *, chtype);
	void *userdata;
	struct dirent **dirents;
	int count;
};

#define MAXSELECTIONS   16
#define MAXSELECTIONLEN 16

struct NcSelection {
	NcList nclist;
	char selections[MAXSELECTIONS][MAXSELECTIONLEN];
	int count;
	char ** value;
	int size;
	int * selected;
	bool multiselect;
	bool focused;
	NCRET (*callback)(NcWidget*, void *, chtype);
	void *userdata;
};

struct NcGroup{
	struct NcGroup *next;
	NcWidget *object;
};

#endif /* ifndef NCWIDGETS_STRUCTURES_H */
