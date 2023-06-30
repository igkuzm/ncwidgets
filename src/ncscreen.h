/**
 * File              : ncscreen.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 14.06.2023
 * Last Modified Date: 30.06.2023
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#ifndef NC_SCREEN_H
#define NC_SCREEN_H

#include <curses.h>
#include "types.h"

typedef struct ncscreen_node{
	struct ncscreen_node *next;
	void *object;
	enum SCREEN type;
} ncscreen_node_t;

ncscreen_node_t *_nc_screen_node_new(enum SCREEN type, void *object);

void nc_screen_add(ncscreen_node_t **root, enum SCREEN type, void *object);

void nc_screen_remove(ncscreen_node_t **root, void *object);

void nc_screen_activate(
		ncscreen_node_t *root,
		void *selected,
		void *userdata,
		CBRET (*callback)(void *userdata, enum SCREEN type, void *object, chtype key)
		);

#endif /* ifndef NC_SCREEN_H */
