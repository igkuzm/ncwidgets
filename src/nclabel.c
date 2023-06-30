/**
 * File              : nclabel.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 14.06.2023
 * Last Modified Date: 30.06.2023
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#include "nclabel.h"
#include "utils.h"
#include "keys.h"

void nc_label_refresh(nclabel_t *nclabel){
	int h, w, y, x;
	getmaxyx(nclabel->ncwin->overlay, h, w);

	// fill with blank 
	for (y = 0; y < h - 2; ++y)
		for (x = 0; x < w - 2; x++)
			mvwaddch(nclabel->ncwin->overlay, y+1, x+1, ' ');
	
	//fill with data
	for (y = 0; y < h - 2 && y < nclabel->lines; ++y) {
		wmove(nclabel->ncwin->overlay, y + 1, 1);		
		u8char_t *str = nclabel->info[y]; 
		
		for (x = 0; x < w - 2 && str[x].utf8[0]; ++x) {
			if (nclabel->focused){
				wattron (nclabel->ncwin->overlay, str[x].attr | A_REVERSE);
				waddstr (nclabel->ncwin->overlay, str[x].utf8);	
				wattroff(nclabel->ncwin->overlay, str[x].attr | A_REVERSE);
			} else{
				wattron (nclabel->ncwin->overlay, str[x].attr);
				waddstr (nclabel->ncwin->overlay, str[x].utf8);	
				wattroff(nclabel->ncwin->overlay, str[x].attr);
			}
		}
	}

	wrefresh(nclabel->ncwin->overlay);
}

nclabel_t * nc_label_new(
		PANEL *parent,
		int y, int x,
		int color,
		const char **value,
		int lines,
		bool box,
		bool shadow
		)
{
	nclabel_t *nclabel = malloc(sizeof(nclabel_t));
	if (!nclabel)
		return NULL;

	nclabel->info = malloc( 8 * lines + 8);
	if (!nclabel->info){
		free(nclabel);
		return NULL;
	}
	
	nclabel->lines = lines;
	nclabel->focused = 0;
	
	/* copy values */
	int i, maxlen = 0;
	for (i = 0; i < nclabel->lines; ++i) {
		nclabel->info[i] = str2ucharstr(value[i], color);
		int len = ucharstrlen(nclabel->info[i]);
		if (len > maxlen)
			maxlen = len;
	}

	int h = lines + 2, w = maxlen + 2;

	nclabel->ncwin = nc_window_new(parent, NULL, h, w, y, x, color, box, shadow);
	if (!nclabel->ncwin){
		for (i = 0; i < nclabel->lines; ++i) {
			free(nclabel->info[i]);
		}
		free(nclabel->info);
		free(nclabel);
		return NULL;
	}

	nc_label_refresh(nclabel);
	
	return nclabel;
}


void nc_label_destroy(nclabel_t *nclabel)
{
	nc_window_destroy(nclabel->ncwin);
	int i;
	for (i = 0; i < nclabel->lines; ++i) {
		free(nclabel->info[i]);
	}
	free(nclabel->info);
	free(nclabel);
}

void nc_label_set_focused(nclabel_t *nclabel, bool focused)
{
	nclabel->focused = focused;
	nc_label_refresh(nclabel);
}

void nc_label_activate(
		nclabel_t *nclabel,
		void *userdata,
		CBRET (*callback)(void *userdata, enum SCREEN type, void *object, chtype key)		
		)
{

	nc_label_set_focused(nclabel, true);

	chtype ch;
	while (ch != CTRL('x')) {
		ch = getch();
		// stop execution if callback not NULL
		if (callback){
			CBRET ret = callback(userdata, SCREEN_nclabel, nclabel, ch);
			if (ret == CBCONTINUE)
				continue;
			else if (ret == CBBREAK)
				break;
		}

	}
}
