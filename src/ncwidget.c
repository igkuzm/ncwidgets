/**
 * File              : ncwidget.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 12.06.2023
 * Last Modified Date: 08.05.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#include "struct.h"

void nc_widget_refresh(NcWidget *widget){
	widget->on_refresh(widget);
}
void nc_widget_set_focused(NcWidget *widget, bool focused){
	widget->on_set_focused(widget, focused);
}
void nc_widget_activate(
		NcWidget *widget, 
		void *userdata,
		NCRET callback(NcWidget *widget, void *userdata, chtype ch)
		)
{
	widget->on_activate(widget, userdata, callback);
}
void nc_widget_destroy(NcWidget *widget){
	widget->on_destroy(widget);
}

int nc_widget_move(NcWidget *widget, int y, int x){
	return nc_win_move(&widget->ncwin, y, x);
}

int nc_widget_resize(NcWidget *widget, int h, int w){
	int ret = nc_win_resize(&widget->ncwin, h, w);
	nc_widget_refresh(widget);
	return ret;
}
