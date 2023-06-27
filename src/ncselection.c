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
	if (selected){
		int *_selected = malloc(size * sizeof(int));
		if (!_selected)
			return;

		for (i = 0; i < size; ++i)
			_selected[i] = selected[i];
		
		if (s->selected)
			free(s->selected);

		s->selected = _selected;
	}

	//set selections
	if (selections){
		s->count = count;
		for (i = 0; i < count; ++i){
			strncpy(s->selections[i], selections[i], MAXSELECTIONLEN - 1);	
			s->selections[i][MAXSELECTIONLEN - 1] = 0;
		}
	}
	
	//set value
	if (value){
		char ** str = malloc(size * 8);
		if (!str)
			return;
		
		char ** v = malloc(size * 8);
		if (!v)
			return;
		
		for (i = 0; i < size; ++i) {
			int len = strlen(value[i]);
			v[i] = malloc(len + 1);
			if (!v[i])
				return;
			strcpy(v[i], value[i]);	

			len += MAXSELECTIONLEN;
			str[i] = malloc(len + 1);
			if (!str[i])
				return;

			strcpy(str[i], s->selections[s->selected[i]]);
			strcat(str[i], value[i]);
		}

		nc_list_set_value(s->nclist, str, size);

		if (s->value){
			for (i = 0; i < s->size; ++i) {
				free(s->value[i]);
			}
			free(s->value);
		}

		s->size = size;

		s->value = v;
	}
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
	s->count = count;
	s->size  = 0;
	s->value = NULL;

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
			s->selected[i] = 0;
			if (i == index)
				s->selected[i] = 1;		
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
	
	if (s->value){
		int i;
		for (i = 0; i < s->size; ++i) {
			free(s->value[i]);
		}
		free(s->value);
	}

	if (s->selected)
		free(s->selected);

}
