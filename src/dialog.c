/**
 * File              : dialog.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 08.05.2024
 * Last Modified Date: 08.05.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#include "keys.h"
#include "ncwidgets.h"
#include "strsplit.h"
#include "struct.h"
#include "utils.h"
#include <ncurses.h>

static NCRET callback(NcWidget *widget, void *data, chtype ch)
{
	int *ret = data;
	if (ch == '\n' || ch == '\r' || ch == KEY_ENTER || ch == KEY_RETURN){
		*ret = widget->key;
		return NCSTOP;
	}
	if (ch == KEY_ESC || ch == 'q'){
		*ret = -1;
		return NCSTOP;
	}

	return NCNONE;
}

int nc_dialog(
		int color,
		const char *title,
		const char *msg, 
		const char *buttons[], int count)
{
	int ret = -1;
	int i, key, x=0, y=0, cols=0, rows=4, alen[count];

	// count lines
	int lines = strsplit(msg, "\n", NULL);
	rows += lines;

	// count cols
	for (i = 0; i < count; ++i) {
		u8char_t *info = str2ucharstr(buttons[i], color);
		int len = ucharstrlen(info);
		free(info);
		alen[i] = len;
		cols += len;
	}
	cols += 3 * count;

	// get center
	int cx = COLS/2 - cols/2;
	int cy = LINES/2 - rows/2;

	NcWin *win = 
		nc_win_new(NULL, title, rows, cols, cy, cx, color, 1, 1);

	nc_label_new(win, 0, 0, color, msg, 0, 0);

	NcGroup *group = nc_group_new();

	for (i = 0; i < count; ++i) {
		NcWidget *b = nc_button_new(
				win, 3, alen[i], rows-3, x, color, buttons[i], 1, 0);
		b->key = i;
		x += alen[i] + 3;
		nc_group_add(group, b);
	}

	nc_group_activate(group, NULL, &ret, callback);

	return ret;
}
