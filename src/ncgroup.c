#include "ncwidgets.h"
#include "keys.h"
#include <curses.h>
#include <ncurses.h>

struct NcGroup{
	struct NcGroup *next;
	NcWidget *object;
};

NcGroup *nc_group_new()
{
	NcGroup *node = malloc(sizeof(NcGroup));
	if (!node)
		return NULL;

	node->object   = NULL;
	node->next     = NULL;
	return node;
}

NcGroup *_nc_group_node_new(NcWidget *object)
{
	NcGroup *node = malloc(sizeof(NcGroup));
	if (!node)
		return NULL;

	node->object   = object;
	node->next     = NULL;
	return node;
}

void nc_group_add(
		NcGroup *root, 
		NcWidget *object)
{
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

NCRET nc_group_cb(NcWidget *widget, void *userdata, chtype key)
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
					ungetmouse(&event);
					}
				}
				break;
	}				
	return 0;
}


void nc_screen_activate(
		ncscreen_node_t *root,
		void *selected,
		void *userdata,
		CBRET (*callback)(void *userdata, enum SCREEN type, void *object, chtype key)
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
