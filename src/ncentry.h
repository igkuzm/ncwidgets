/**
 * File              : ncentry.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 16.06.2023
 * Last Modified Date: 26.06.2023
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#ifndef NC_ENTRY_H
#define NC_ENTRY_H

#include <curses.h>
#include <ncurses.h>
#include <stddef.h>
#include "keys.h"
#include "types.h"
#include "utils.h"
#include "ncwin.h"

typedef struct ncentry {
	ncwin_t *ncwin;
	u8char_t *info;
	size_t allocated;
	bool multiline;
	size_t position;
	int ypos;	
	int xpos;	
	bool focused;
} ncentry_t;

ncentry_t * nc_entry_new(
		PANEL *parent,
		const char *title,
		int h, int w, int y, int x,
		int color,
		const char *value,
		bool multiline,
		bool box,
		bool shadow
		);

void nc_entry_set_value(
		ncentry_t *ncentry,
		const char *value
		);

void nc_enrty_refresh(ncentry_t *ncentry);

void nc_entry_destroy(ncentry_t *ncentry);

void nc_entry_set_position(ncentry_t *ncentry, size_t position);
size_t nc_entry_get_position(ncentry_t *ncentry);

void nc_entry_set_focused(ncentry_t *ncentry, bool focused);
#define nc_set_focused_ncentry(object, focused)\
		nc_entry_set_focused(object, focused)

void nc_entry_activate(
		ncentry_t *ncentry,
		void *userdata,
		CBRET (*callback)(void *userdata, enum SCREEN type, void *object, chtype key)
		);

#define nc_activate_ncentry(object, data, callback)\
		nc_entry_activate(object, data, callback)

#endif /* ifndef NC_ENTRY_H */
