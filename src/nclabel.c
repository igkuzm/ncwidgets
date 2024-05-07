/**
 * File              : nclabel.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 14.06.2023
 * Last Modified Date: 08.05.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#include "ncwidgets.h"
#include "struct.h"
#include "utils.h"
#include "keys.h"
#include "strsplit.h"

void nc_label_refresh(NcWidget *ncwidget){
	NcLabel *nclabel = (NcLabel *)ncwidget;
	int h, w, y, x;
	getmaxyx(nclabel->ncwidget.ncwin.overlay, h, w);

	// fill with blank 
	for (y = 0; y < h - 2; ++y)
		for (x = 0; x < w - 2; x++)
			mvwaddch(nclabel->ncwidget.ncwin.overlay, y+1, x+1, ' ');
	
	//fill with data
	for (y = 0; y < h - 2 && y < nclabel->lines; ++y) {
		wmove(nclabel->ncwidget.ncwin.overlay, y + 1, 1);		
		u8char_t *str = nclabel->info[y]; 
		
		for (x = 0; x < w - 2 && str[x].utf8[0]; ++x) {
			if (nclabel->ncwidget.focused){
				wattron (nclabel->ncwidget.ncwin.overlay, str[x].attr | A_REVERSE);
				waddstr (nclabel->ncwidget.ncwin.overlay, str[x].utf8);	
				wattroff(nclabel->ncwidget.ncwin.overlay, str[x].attr | A_REVERSE);
			} else{
				wattron (nclabel->ncwidget.ncwin.overlay, str[x].attr);
				waddstr (nclabel->ncwidget.ncwin.overlay, str[x].utf8);	
				wattroff(nclabel->ncwidget.ncwin.overlay, str[x].attr);
			}
		}
	}

	wrefresh(nclabel->ncwidget.ncwin.overlay);
}

void nc_label_destroy(NcWidget *ncwidget)
{
	NcLabel *nclabel = (NcLabel *)ncwidget;
	nc_win_destroy(&nclabel->ncwidget.ncwin);
	int i;
	for (i = 0; i < nclabel->lines; ++i) {
		free(nclabel->info[i]);
	}
	free(nclabel->info);
	free(nclabel);
}

void nc_label_set_focused(NcWidget *ncwidget, bool focused)
{
	NcLabel *nclabel = (NcLabel *)ncwidget;
	nclabel->ncwidget.focused = focused;
	nc_win_activate(&ncwidget->ncwin);
	nc_label_refresh(ncwidget);
}

void nc_label_activate(
		NcWidget *ncwidget,
		void *userdata,
		NCRET (*callback)(NcWidget *, void *, chtype)		
		)
{
	NcLabel *nclabel = (NcLabel *)ncwidget;

	nc_label_set_focused(ncwidget, true);

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
	}

	nc_widget_set_focused(ncwidget, false);
	nc_widget_refresh(ncwidget);
}

NcWidget * nc_label_new(
		NcWin *parent,
		int y, int x,
		int color,
		const char *text,
		bool box,
		bool shadow
		)
{
	// get number of lines
	char **tokens;
	int lines = 
		strsplit(text, "\n", &tokens);
	
	// allocate multiline info
	u8char_t **info;
	info = malloc( 8 * lines + 8);
	if (!info)
		return NULL;

	/* copy values */
	int i, maxlen = 0;
	for (i = 0; i < lines; ++i) {
		info[i] = str2ucharstr(tokens[i], color);
		int len = ucharstrlen(info[i]);
		if (len > maxlen)
			maxlen = len;
	}
	
	int h = lines + 2, w = maxlen + 2;
	
	NcLabel *nclabel =
		(NcLabel *)nc_win_new(parent, NULL, h, w, y, x, color, box, shadow);
	if (!nclabel)
		return NULL;

	nclabel = realloc(nclabel, sizeof(NcLabel));
	if (!nclabel)
		return NULL;
	
	nclabel->ncwidget.type = NcWidgetTypeLabel;

	nclabel->info = info;
	nclabel->lines = lines;
	nclabel->ncwidget.focused = 0;
	
	// free tokens
	free(tokens);

	nc_label_refresh((NcWidget *)nclabel);
	nclabel->ncwidget.on_refresh     = nc_label_refresh;
	nclabel->ncwidget.on_set_focused = nc_label_set_focused;
	nclabel->ncwidget.on_activate    = nc_label_activate;
	nclabel->ncwidget.on_destroy	   = nc_label_destroy;
	
	return (NcWidget*)nclabel;
}
