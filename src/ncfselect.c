/**
 * File              : ncfselect.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 30.06.2023
 * Last Modified Date: 01.07.2023
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#include "ncfselect.h"

#include "keys.h"
#include "ls.h"
#include "types.h"
#include "utils.h"
#include "nclist.h"
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static int ls_callback(const char *filename, FILETYPE type, void *user_data){
	int i;
	ncfselect_t *s = user_data;
	
	// copy filename
	s->nclist->info[s->nclist->size] =
			str2ucharstr(filename, s->nclist->ncwin->color);
	
	// set attributes
	//attr_t attr;
	//switch (type) {
		//case FILETYPE_FILE:
			//attr = s->file_attr;
			//break;
		//case FILETYPE_DIR:
			//attr = s->dir_attr;
			//break;
		//case FILETYPE_LINK:
			//attr = s->link_attr;
			//break;
		//case FILETYPE_OTHER:
			//attr = s->other_attr;
			//break;
	//}	

	//for (i = 0; s->nclist->info[s->nclist->size][i].utf8[0]; ++i)
		//s->nclist->info[s->nclist->size][i].attr = attr;	

	// set filetype 
	s->files.types[s->nclist->size] = type;

	// realloc arrays
	void *ptr;
	ptr = realloc(s->nclist->info, s->nclist->size * 8 + 2 * 8);
	if (!ptr)
		return 0;
	s->nclist->info    = ptr;
	s->files.filenames = s->nclist->info;

	ptr = realloc(s->files.types, s->nclist->size * sizeof(FILETYPE) + 2 * sizeof(FILETYPE));
	if (!ptr)
		return 0;
	s->files.types = ptr;	

	s->nclist->size++;

	return 0;
}

void nc_fselect_refresh(ncfselect_t *s)
{
	int i;

	// free old types
	if (s->files.types)
		free(s->files.types);	

	// free old info
	for (i = 0; i < s->nclist->size; ++i)
		free(s->nclist->info[i]);
	if(s->nclist->info)
		free(s->nclist->info);
	
	s->nclist->size = 0;
	s->files.count = &s->nclist->size;

	// allocate new info and file types
	s->nclist->info = malloc(8);
	if (!s->nclist->info)
		return;
	s->files.filenames = s->nclist->info;

	s->files.types = malloc(sizeof(FILETYPE));
	if (!s->files.types)
		return;

	// list directory 
	ls(s->path, s, ls_callback);

	// set list selected to 0 
	s->nclist->selected = 0;

	nc_list_refresh(s->nclist);
}

void nc_fselect_set_focused(ncfselect_t *s, bool focused){
	nc_list_set_focused(s->nclist, focused);
}

void nc_fselect_set_path(ncfselect_t *s, const char *path){
	strcat(s->path, "/");
	strncat(s->path, path, BUFSIZ - 1);
	s->path[BUFSIZ - 1] = 0;

	nc_fselect_refresh(s);
}

ncfselect_t *
nc_fselect_new(
		PANEL *parent, 
		const char *title, 
		int h, int w, int y, int x, 
		int color, 
		const char *path,
		attr_t file_attr,
		attr_t dir_attr,
		attr_t link_attr,
		attr_t other_attr,		
		bool box, 
		bool shadow
		)
{
	ncfselect_t *s = malloc(sizeof(ncfselect_t));
	if (!s)
		return NULL;

	s->nclist = nc_list_new(parent, title, h, w, y, x, color, NULL, 0, box, shadow);
	if (!s->nclist){
		free(s);
		return NULL;
	}

	s->files.filenames = NULL;
	s->files.types     = NULL;
	
	s->ncwin = s->nclist->ncwin;

	s->file_attr = file_attr;
	s->dir_attr  = dir_attr;
	s->link_attr = link_attr;
	s->other_attr= other_attr;

	nc_fselect_set_path(s, path);

	return s;
}

static CBRET 
nc_fselect_activate_callback(void *userdata, enum SCREEN type, void *object, chtype key){
	ncfselect_t *s = userdata;

	if (s->callback){
		CBRET ret = s->callback(userdata, SCREEN_nclist, s, key);
		if (ret)
			return ret;
	}

	switch (key) {
		case KEY_ENTER: case '\n': case '\r': case KEY_SPACE:
			{
				int index = nc_list_get_selected(s->nclist);
				if (s->files.types[index] == FILETYPE_DIR){
					char *path = ucharstr2str(s->files.filenames[index]);
					nc_fselect_set_path(s, path);
					free(path);
					return CBCONTINUE;
				}
			}

		defaut: 
			break;
	}	

	return 0;	
}

void
nc_fselect_activate(
		ncfselect_t *s, 
		void *userdata, 
		CBRET (*callback)(void *userdata, enum SCREEN type, void *object, chtype key)
		)
{
	s->userdata = userdata;
	s->callback = callback;
	nc_list_activate(s->nclist, s, nc_fselect_activate_callback);
}
