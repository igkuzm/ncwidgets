/**
 * File              : ncselection.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 27.06.2023
 * Last Modified Date: 27.06.2023
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#ifndef NC_SELECTION_H
#define NC_SELECTION_H

#include "utils.h"
#include "nclist.h"

#define MAXSELECTIONS   16
#define MAXSELECTIONLEN 16

typedef struct ncselection {
	nclist_t * nclist;
	char selections[MAXSELECTIONS][MAXSELECTIONLEN];
	int count;
	char ** value;
	int size;
	int * selected;
	bool multiselect;
	CBRET (*callback)(void *, enum SCREEN, void *, chtype);
	void *userdata;
} ncselection_t;

ncselection_t *
nc_selection_new(
		PANEL *parent, 
		const char *title, 
		int h, int w, int y, int x, 
		int color, 
		char **selections,
		int count,
		int * selected,
		bool multiselect,
		char **value, 
		int size, 
		bool box, 
		bool shadow
		);

void nc_selection_set(
		ncselection_t *s, 
		char **value, 
		int size, 
		char **selections, 
		int count,
		int * selected
		);

void  nc_selection_select(ncselection_t *s, int index, int selected);

void
nc_selection_activate(
		ncselection_t *s, 
		void *userdata, 
		CBRET (*callback)(void *userdata, enum SCREEN type, void *object, chtype key)
		);

void  nc_selection_destroy(ncselection_t *s);

#endif /* ifndef NC_SELECTION_H */			
