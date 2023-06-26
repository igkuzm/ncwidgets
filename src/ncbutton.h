/**
 * File              : ncbutton.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 14.06.2023
 * Last Modified Date: 26.06.2023
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#ifndef NC_BUTTON_H
#define NC_BUTTON_H

#include "ncwin.h"
#include "utils.h"
#include "types.h"
#include <curses.h>

typedef struct ncbutton {
	ncwin_t *ncwin;
	u8char_t **info;
	int lines;
	bool focused;
} ncbutton_t;

ncbutton_t * nc_button_new(
		PANEL *parent,
		int h, int w, int y, int x,
		int color,
		const char **value,
		int lines,
		bool box,
		bool shadow
		);

void nc_button_refresh(ncbutton_t *ncbutton);
void nc_button_destroy(ncbutton_t *ncbutton);

void nc_button_set_focused(ncbutton_t *ncbutton, bool focused);
#define nc_set_focused_ncbutton(object, focused)\
		nc_button_set_focused(object, focused)

void nc_button_activate(
		ncbutton_t *ncbutton,
		void *userdata,
		CBRET (*callback)(void *userdata, enum SCREEN type, void *object, chtype key)
		);

#define nc_activate_ncbutton(object, data, callback)\
		nc_button_activate(object, data, callback)

#endif /* ifndef NC_BUTTON_H */
