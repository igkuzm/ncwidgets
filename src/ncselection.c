/**
 * File              : ncselection.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 27.06.2023
 * Last Modified Date: 27.06.2023
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#include "ncselection.h"
#include "keys.h"
#include "types.h"
#include "utils.h"
#include "nclist.h"
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void nc_selection_set_selected(ncselection_t *s, int size, int *selected)
{
	int i;

	int *v = malloc(size * sizeof(int));
	if (!v)
		return;

	for (i = 0; i < size; ++i)
		v[i] = selected[i];
		
	if (s->selected)
		free(s->selected);

	s->selected = v;
}

void nc_selection_set_selections(ncselection_t *s, int count, char **selections)
{
	int i;
	for (i = 0; i < count; ++i){
		strncpy(s->selections[i], selections[i], MAXSELECTIONLEN - 1);	
		s->selections[i][MAXSELECTIONLEN - 1] = 0;
	}
	s->count = count;
}

void nc_selection_set_value(ncselection_t *s, int size, char **value)
{
	int i;

	// free old info
	for (i = 0; i < s->nclist->size; ++i)
		free(s->nclist->info[i]);
	free(s->nclist->info);

	// allocate new info
	s->nclist->info = malloc( 8 * size + 8);
	if (!s->nclist->info)
		return;
	s->nclist->size = size;

	// allocate value
	char **v = malloc( 8 * size);
	if (!v)
		return;

	// copy values
	for (i = 0; i < size; ++i) {
		int len = strlen(value[i]);
		v[i] = malloc(len + 1);
		if (!v[i])
			return;
		strcpy(v[i], value[i]);	

		len += MAXSELECTIONLEN;
		char *str = malloc(len + 1);
		if (!str)
			return;

		strcpy(str, s->selections[s->selected[i]]);
		strcat(str, value[i]);
	
		s->nclist->info[i] = str2ucharstr(str, s->nclist->ncwin->color);

		free(str);
	}

	nc_list_refresh(s->nclist);

	// free old values
	/*
	if (s->value){
		for (i = 0; i < s->size; ++i)
			free(s->value[i]);
		free(s->value);
	}
	*/

	// set args
	s->value = v;
	s->size  = size;
}

void nc_selection_set(
		ncselection_t *s, 
		char **value, 
		int size, 
		char **selections, 
		int count,
		int * selected
		)
{	
	
	int i;

	//set selected
	if (selected)
		nc_selection_set_selected(s, size, selected);
		
	//set selections
	if (selections)
		nc_selection_set_selections(s, count, selections);
	
	//set value
	if (value)
		nc_selection_set_value(s, size, value);
}

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
		)
{
	ncselection_t *s = malloc(sizeof(ncselection_t));
	if (!s)
		return NULL;

	s->nclist = nc_list_new(parent, title, h, w, y, x, color, NULL, 0, box, shadow);
	if (!s->nclist){
		free(s);
		return NULL;
	}
	
	s->multiselect = multiselect;
	s->count = 0;
	s->size  = 0;
	s->value = NULL;
	s->selected = NULL;

	nc_selection_set(s, value, size, selections, count, selected);

	return s;
}

CBRET nc_selection_callback(void *userdata, enum SCREEN type, void *object, chtype key)
{
	ncselection_t *s = userdata;
	
	if (s->callback){
		CBRET ret = s->callback(s->userdata, type, object, key);
		if (ret)
			return ret;
	}
	
	switch (key) {
		case KEY_ENTER: case '\n': case '\r': case KEY_SPACE:
			{
				int index = s->nclist->selected;
				int selected = s->selected[index];
				selected++;
				if (selected >= s->count)
					selected = 0;
				nc_selection_select(s, index, selected);
				return CBCONTINUE;
			}

		defaut: 
			break;
	}
	
	return 0;
}

void 
nc_selection_select(ncselection_t *s, int index, int selected){
	if (s->multiselect)
		s->selected[index] = selected;
	else {
		int i;
		for (i = 0; i < s->size; ++i) {
			if (i == index)
				s->selected[i] = 1;		
			else 
				s->selected[i] = 0;
		}
	}

	nc_selection_set(s, s->value, s->size, NULL, 0, s->selected);
}

void
nc_selection_activate(
		ncselection_t *s, 
		void *userdata, 
		CBRET (*callback)(void *userdata, enum SCREEN type, void *object, chtype key)
		)
{
	s->callback = callback;
	s->userdata = userdata;
	nc_list_activate(s->nclist, s, nc_selection_callback);
}

void  nc_selection_destroy(ncselection_t *s){
	nc_list_destroy(s->nclist);
	
	//if (s->value){
		//int i;
		//for (i = 0; i < s->size; ++i) {
			//free(s->value[i]);
		//}
		//free(s->value);
	//}

	if (s->selected)
		free(s->selected);

	free(s);
}
