/**
 * File              : ncfselect.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 08.05.2024
 * Last Modified Date: 08.05.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */
#include "colors.h"
#include "ncwidgets.h"
#include "struct.h"
#include "fm.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#define SLASH "\\"
#else
#define SLASH "/"
#endif

static char * staticpath = NULL;

static bool is_dir(const char *path, const struct dirent *d){
	if (d->d_type == DT_DIR)
		return true;
	char filepath[BUFSIZ];
	snprintf(filepath, BUFSIZ-1,
		 	"%s" SLASH "%s", path, d->d_name);
	DIR *dir;
	if ((dir = opendir(filepath))){
		closedir(dir);
		return true;
	}
	return false;
}

static int file_compar(
		const struct dirent **a, 
		const struct dirent **b)
{
	// check if dir
	if (
			is_dir(staticpath, *a) && 
			!is_dir(staticpath, *b)
			)
		return -1;
	if (
			!is_dir(staticpath, *a) && 
			is_dir(staticpath, *b))
		return 1;
		
	return strcoll((*a)->d_name, (*b)->d_name);
}

static int 
file_select_filter(const struct dirent *d){
	// no names start with dot
	if (d->d_name[0] == '.'){
		if (d->d_name[1] == '.'){
			if (strcmp(staticpath, SLASH))
				return 1;
		}
		return 0;
	}
	return 1;
}

void nc_fselect_set_value(NcFselect *fselect)
{
	fselect->nclist.info = 
		malloc( 8 * fselect->count + 8);
	if (!fselect->nclist.info){
		return;
	}
	fselect->nclist.size = fselect->count;
	
	/* copy values */
	int i;
	for (i = 0; i < fselect->count; ++i) {
		attr_t color;
		switch (fselect->dirents[i]->d_type) {
			case DT_REG:
				color = fselect->file_attr;
				break;
			case DT_DIR:
				color = fselect->dir_attr;
				break;
			case DT_LNK:
				color = fselect->link_attr;
				break;
			default:
				color = fselect->other_attr;
				break;
		}
		fselect->nclist.info[i] = 
			str2ucharstr(fselect->dirents[i]->d_name, 
					color);
	}

	nc_widget_refresh((NcWidget*)fselect);
}

void nc_fselect_refresh(NcFselect *fselect, int selected)
{
	staticpath = fselect->path;

	fselect->count = scandir(
			fselect->path, 
			&fselect->dirents,
		 	file_select_filter, 
			file_compar);

	// reload list
	nc_fselect_set_value(fselect);
	nc_list_set_selected(&fselect->nclist, selected);
}

void nc_fselect_set(NcFselect *fselect, const char *path){
	strncpy(fselect->path, path, BUFSIZ - 1);
	fselect->path[BUFSIZ-1] = 0;
	nc_fselect_refresh(fselect, 0);
}

const struct dirent * nc_fselect_get(NcFselect *fselect){
	int i = nc_list_get_selected(&fselect->nclist);
	return fselect->dirents[i];
}

NCRET nc_fselect_callback(
		NcWidget *widget, void *userdata, chtype key)
{
	NcFselect *fselect = (NcFselect *)widget;
	
	if (fselect->callback){
		NCRET ret = fselect->callback(
				widget, fselect->userdata, key);
		if (ret)
			return ret;
	}
	
	switch (key) {
		case KEY_ENTER: case '\n': case '\r':
			{
				int selected = 
						nc_list_get_selected(&fselect->nclist);
				if (selected >= 0){
					if (fselect->dirents[selected]->d_type == DT_DIR){
						if (strcmp(fselect->dirents[selected]->d_name, "..")){
							strcat(fselect->path, SLASH);
							strcat(fselect->path,
									fselect->dirents[selected]->d_name);
							selected = 0;
						} else {
							int i = lastpath(fselect->path);
							fselect->path[i] = 0;
						}
						nc_fselect_refresh(fselect, selected);
					}
				}
				return NCCONT;
			}

		defaut: 
			break;
	}
	
	return 0;
}

void nc_fselect_activate(
		NcWidget *ncwidget, 
		void *userdata, 
		NCRET (*callback)(NcWidget *, void *, chtype)
		)
{
	NcFselect *s = (NcFselect *)ncwidget;
	s->callback = callback;
	s->userdata = userdata;
	nc_list_activate(ncwidget, userdata, 
			nc_fselect_callback);
}

NcWidget *nc_fselect_new(
		NcWin *parent, 
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
	NcWidget *widget = 
		nc_list_new(parent, title, h, w, y, x, color,
			 	NULL, 0, box, shadow);
	if (!widget)
		return NULL;

	// realoc
	widget = realloc(widget, sizeof(NcFselect));
	if (!widget)
		return NULL;
	
	widget->type = NcWidgetTypeFselect;

	// copy path to structure
	NcFselect *fselect = (NcFselect *)widget;
	strncpy(fselect->path, path, BUFSIZ - 1);
	fselect->path[BUFSIZ-1] = 0;

	fselect->file_attr  = file_attr;
	fselect->dir_attr   = dir_attr;
	fselect->link_attr  = link_attr;
	fselect->other_attr = other_attr;

	// get file list
	nc_fselect_refresh((NcFselect*)widget, 0);

	widget->on_activate = nc_fselect_activate;
	
	return widget;
}
