/**
 * File              : ncscreen.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 14.06.2023
 * Last Modified Date: 21.06.2023
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#ifndef NC_SCREEN_H
#define NC_SCREEN_H

#include <curses.h>
#include <stddef.h>
#include <stdlib.h>
#include "keys.h"
#include "types.h"
#include "ncwin.h"
#include "nclist.h"
#include "nclabel.h"
#include "ncbutton.h"
#include "ncentry.h"

typedef struct ncscreen_node{
	struct ncscreen_node *next;
	void *object;
	enum SCREEN type;
} ncscreen_node_t;

static ncscreen_node_t *_nc_screen_node_new(enum SCREEN type, void *object)
{
	ncscreen_node_t *node = malloc(sizeof(ncscreen_node_t));
	if (!node)
		return NULL;

	node->type     = type;
	node->object   = object;
	node->next     = NULL;
	return node;
}

static void nc_screen_add(
		ncscreen_node_t **root, 
		enum SCREEN type, 
		void *object)
{
	// get last node
	ncscreen_node_t *ptr = *root;
	while (ptr && ptr->next)
		ptr = ptr->next;	

	// create node
	ncscreen_node_t *node = _nc_screen_node_new(type, object);	
	if (ptr)
		ptr->next = node;
	else
		*root = node;
}

static void nc_screen_remove(
		ncscreen_node_t **root, 
		void *object)
{
	// get node
	ncscreen_node_t *ptr  = *root;
	ncscreen_node_t *prev = NULL;
	while (ptr){
		if (ptr->object == object){
			prev->next = ptr->next;
			break;
		}
		
		prev = ptr;
		ptr = ptr->next;	
	}
}

struct nc_screen_data {
	ncscreen_node_t *root;
	ncscreen_node_t *ptr;
	void *userdata;
	int (*callback)(void *userdata, enum SCREEN type, void *object, chtype key);
};

static int nc_screen_cb(void *userdata, enum SCREEN type, void *object, chtype key)
{
	struct nc_screen_data *d = userdata;
	if (d->callback)
		if (d->callback(d->userdata, type, object, key))
			return 1;

	//switch keys
	switch (key) {
		// next object
		case KEY_TAB:
			{
				switch (d->ptr->type) {
#define NCSCREEN(title)\
					case SCREEN_##title:\
						   nc_set_focused_##title((struct title *)(d->ptr->object),\
								   false);\
					break;
	NCSCREENS
				}
#undef NCSCREEN					
				if (d->ptr->next)
					d->ptr = d->ptr->next;
				else
					d->ptr = d->root;

				switch (d->ptr->type) {
#define NCSCREEN(title)\
					case SCREEN_##title:\
						nc_activate_##title((struct title *)(d->ptr->object),\
								d, nc_screen_cb);\
						return 1;
	NCSCREENS
					}
#undef NCSCREEN					
				break;
			}
		case KEY_MOUSE:
			{
				MEVENT event;
				if (getmouse(&event) == OK) {
					if (event.bstate & BUTTON1_PRESSED){
						ncwin_t *win = NULL;
						switch (d->ptr->type) {
#define NCSCREEN(title)\
							case SCREEN_##title:{\
								win = ((struct title *)(d->ptr->object))->ncwin;\
								if (!wenclose(win->overlay, event.y, event.x)){\
									ncscreen_node_t *prev = d->ptr;\
									nc_set_focused_##title((struct title *)(d->ptr->object),\
											false);\
									d->ptr = d->root;\
									while(d->ptr){\
										win = ((struct title *)(d->ptr->object))->ncwin;\
										if (wenclose(win->overlay,\
													event.y, event.x)){\
											nc_activate_##title((struct title *)(d->ptr->object),\
												d, nc_screen_cb);\
											return 1;\
										}\
										d->ptr = d->ptr->next;\
									}\
									d->ptr = prev;\
									nc_activate_##title((struct title *)(d->ptr->object),\
										d, nc_screen_cb);\
									return 1;\
								}\
							}\
							break;
NCSCREENS
#undef NCSCREEN
							}						
						}
					}
				}
				break;
	}				
	return 0;
}


static void nc_screen_activate(
		ncscreen_node_t *root,
		void *selected,
		void *userdata,
		int (*callback)(void *userdata, enum SCREEN type, void *object, chtype key)
		)
{
	// ptr is selected object in screen
	ncscreen_node_t *ptr = root;
	if (selected){
		while (ptr->next){
			if (ptr->object == selected)
				break;
			ptr = ptr->next;
		} 
	}
	struct nc_screen_data d = {root, ptr, userdata, callback};

	switch (ptr->type) {
#define NCSCREEN(title)\
		case SCREEN_##title:\
			nc_activate_##title((struct title *)(ptr->object),\
					&d, nc_screen_cb);\
			break;
	NCSCREENS
#undef NCSCREEN					
	}
}

#endif /* ifndef NC_SCREEN_H */
