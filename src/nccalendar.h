/**
 * File              : nccalendar.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 29.06.2023
 * Last Modified Date: 30.06.2023
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#ifndef NC_CALENDAR_H
#define NC_CALENDAR_H

#include "ncwin.h"
#include "utils.h"
#include "types.h"
#include <curses.h>
#include <time.h>

enum nccalendar_selected{
	nccalendar_selected_day,
	nccalendar_selected_month,
	nccalendar_selected_year,
};

typedef struct nccalendar {
	ncwin_t *ncwin;
	struct tm *tm;
	int startofweek;
	bool focused;
	enum nccalendar_selected selected;
} nccalendar_t;

nccalendar_t * nc_calendar_new(
		PANEL *parent,
		const char *title,
		int y, int x,
		int color,
		time_t time,
		int startofweek,
		bool box,
		bool shadow
		);

void     nc_calendar_set(nccalendar_t *nccalendar, time_t time);
time_t * nc_calendar_get(nccalendar_t *nccalendar);

void nc_calendar_refresh(nccalendar_t *nccalendar);

void nc_calendar_destroy(nccalendar_t *nccalendar);

void nc_calendar_set_focused(nccalendar_t *nccalendar, bool focused);
#define nc_set_focused_nccalendar(object, focused)\
		nc_calendar_set_focused(object, focused)

void nc_calendar_activate(
		nccalendar_t *nccalendar,
		void *userdata,
		CBRET (*callback)(void *userdata, enum SCREEN type, void *object, chtype key)
		);

#define nc_activate_nccalendar(object, data, callback)\
		nc_calendar_activate(object, data, callback)


#endif /* ifndef NC_CALENDAR_H */
