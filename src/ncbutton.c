/**
 * File              : ncbutton.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 14.06.2023
 * Last Modified Date: 09.05.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#include "ncwidgets.h"
#include "struct.h"
#include "keys.h"

void nc_button_activate(
		NcWidget *ncwidget,
		void *userdata,
		NCRET (*callback)(NcWidget *, void *, chtype)		
		)
{
	NcButton *ncbutton = (NcButton *)ncwidget;
	nc_widget_set_focused(ncwidget, true);

	chtype ch;
	while (ch != CTRL('x')) {
		ch = getch();
		// stop execution if callback not NULL
		if (callback){
			NCRET ret = callback(ncwidget, userdata, ch);
			if (ret == NCSTOP)
				break;
			else if (ret == NCCONT)
				continue;
		}

		//switch keys
		switch (ch) {
			case KEY_MOUSE:
				{
					MEVENT event;
					if (nc_getmouse(&event) == OK) {
						if (wenclose(ncbutton->ncwidget.ncwin.overlay, event.y, event.x)){
							if (event.bstate & BUTTON1_PRESSED){
								if (callback)
									if(callback(ncwidget, userdata, KEY_RETURN))
										return;
							}
						}
					}
					break;
				}

			default:
				beep();
				break;
		}
	}
}

NcWidget * nc_button_new(
		NcWin *parent,
		int h, int w, int y, int x,
		int color,
		const char *text,
		bool box,
		bool shadow
		)
{
	NcWidget *ncbutton = 
		nc_label_new(parent, y, x, color, text, box, shadow);
	if (!ncbutton)
		return NULL;

	ncbutton->type = NcWidgetTypeButton;
	ncbutton->on_activate = nc_button_activate;

	nc_win_resize(&ncbutton->ncwin, h, w);

	nc_widget_refresh(ncbutton);
	
	return (NcWidget *)ncbutton;
}
