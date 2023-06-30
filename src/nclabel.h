/**
 * File              : nclabel.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 14.06.2023
 * Last Modified Date: 30.06.2023
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#ifndef NC_LABEL_H
#define NC_LABEL_H

#include "ncwin.h"
#include "utils.h"
#include "types.h"
#include <curses.h>

typedef struct nclabel {
	ncwin_t *ncwin;
	u8char_t **info;
	int lines;
	bool focused;
} nclabel_t;

nclabel_t * nc_label_new(
		PANEL *parent,
		int y, int x,
		int color,
		const char **value,
		int lines,
		bool box,
		bool shadow
		);

void nc_label_set_focused(nclabel_t *nclabel, bool focused);
#define nc_set_focused_nclabel(obj, focused)\
	nc_label_set_focused(obj, focused)

void nc_label_refresh(nclabel_t *nclabel);

void nc_label_activate(
		nclabel_t *nclabel,
		void *userdata,
		CBRET (*callback)(void *userdata, enum SCREEN type, void *object, chtype key)
		);

#define nc_activate_nclabel(object, data, callback)\
		nc_label_activate(object, data, callback)

void nc_label_destroy(nclabel_t *nclabel);

#endif /* ifndef NC_LABEL_H */
