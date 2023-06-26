/**
 * File              : nclist.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 12.06.2023
 * Last Modified Date: 26.06.2023
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#ifndef NC_LIST_H
#define NC_LIST_H

#include "ncwin.h"
#include "utils.h"
#include "types.h"
#include <curses.h>

typedef struct nclist {
	ncwin_t *ncwin;
	u8char_t **info;
	int size;
	int selected;
	int ypos;	
	int xpos;	
	bool focused;
} nclist_t;

nclist_t * nc_list_new(
		PANEL *parent,
		const char *title,
		int h, int w, int y, int x,
		int color,
		const char **value,
		int size,
		bool box,
		bool shadow
		);

void nc_list_set_value(
		nclist_t *nclist,
		const char **value,
		int size		
		);

void nc_list_refresh(nclist_t *nclist);

void nc_list_destroy(nclist_t *nclist);

void nc_list_set_selected(nclist_t *nclist, int index);
int nc_list_get_selected(nclist_t *nclist);

void nc_list_set_focused(nclist_t *nclist, bool focused);
#define nc_set_focused_nclist(object, focused)\
		nc_list_set_focused(object, focused)

void nc_list_activate(
		nclist_t *nclist,
		void *userdata,
		CBRET (*callback)(void *userdata, enum SCREEN type, void *object, chtype key)
		);

#define nc_activate_nclist(object, data, callback)\
		nc_list_activate(object, data, callback)

#endif /* ifndef NC_LIST_H */
