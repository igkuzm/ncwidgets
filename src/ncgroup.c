/**
 * File              : ncgroup.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 08.05.2024
 * Last Modified Date: 08.05.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */
#include "ncwidgets.h"
#include "struct.h"
#include "keys.h"
#include <curses.h>
#include <ncurses.h>

NcGroup *_nc_group_node_new(NcWidget *object)
{
	NcGroup *node = malloc(sizeof(NcGroup));
	if (!node)
		return NULL;

	node->object   = object;
	node->next     = NULL;
	return node;
}

NcGroup *nc_group_new()
{
	return _nc_group_node_new(NULL);
}

void nc_group_add(
		NcGroup *root, 
		NcWidget *object)
{
	// if first 
	if (root && root->object == NULL){
		root->object = object;
		return;
	}

	// get last node
	NcGroup *ptr = root;
	while (ptr && ptr->next)
		ptr = ptr->next;	

	// create node
	NcGroup *node = _nc_group_node_new(object);	
	ptr->next = node;
}

void nc_group_remove(
		NcGroup *root, 
		NcWidget *object)
{
	// get node
	NcGroup *ptr  = root;
	NcGroup *prev = NULL;
	while (ptr){
		if (ptr->object == object){
			prev->next = ptr->next;
			break;
		}
		
		prev = ptr;
		ptr = ptr->next;	
	}
}

struct nc_group_data {
	NcGroup *root;
	NcGroup *ptr;
	void *userdata;
	NCRET (*callback)(NcWidget *, void *, chtype);
};

NCRET 
nc_group_cb(NcWidget *widget, void *userdata, chtype key)
{
	struct nc_group_data *d = userdata;
	if (d->callback){
		NCRET ret = d->callback(widget, d->userdata, key);
			if (ret == NCSTOP)
				return ret;
	}

	//switch keys
	switch (key) {
		// next object
		case KEY_TAB:
			{
				nc_widget_set_focused(d->ptr->object, 
						false);
				if (d->ptr->next)
					d->ptr = d->ptr->next;
				else
					d->ptr = d->root;
				nc_widget_activate(d->ptr->object, 
						d, nc_group_cb);
				return NCSTOP;
			}
		case KEY_MOUSE:
			{
				MEVENT event;
				if (getmouse(&event) == OK) {
					if (event.bstate & BUTTON1_PRESSED){
						NcWin *win = NULL;
						win = &d->ptr->object->ncwin;
						if (!wenclose(win->overlay, event.y, event.x))
						{
							NcGroup *prev = d->ptr;
							nc_widget_set_focused(d->ptr->object, 
									false);
							d->ptr = d->root;
							while(d->ptr){
								win = &d->ptr->object->ncwin;
								if (wenclose(win->overlay,
											event.y, event.x))
								{
									nc_widget_activate(d->ptr->object,
										 	d, nc_group_cb);
									return NCSTOP;
								}
								d->ptr = d->ptr->next;
							}
							d->ptr = prev;
							nc_widget_activate(d->ptr->object,
									d, nc_group_cb);
							return NCSTOP;
						}
						break;
					}
					ungetmouse(&event);
				}
			}
			break;
	}				
	return NCNONE;
}

void nc_group_activate(
		NcGroup *root,
		NcWidget *selected,
		void *userdata,
		NCRET (*callback)(NcWidget *, void *, chtype)
		)
{
	// ptr is selected object in screen
	NcGroup *ptr = root;
	if (selected){
		while (ptr->next){
			if (ptr->object == selected)
				break;
			ptr = ptr->next;
		} 
	}
	struct nc_group_data d = 
	{root, ptr, userdata,
	 	callback};

	nc_widget_activate(ptr->object, 
			&d, nc_group_cb);
}
