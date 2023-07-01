/**
 * File              : ncfselect.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 30.06.2023
 * Last Modified Date: 01.07.2023
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#ifndef NC_FSELECT_H
#define NC_FSELECT_H

#include "ncwin.h"
#include "utils.h"
#include "nclist.h"

struct ncfselect_files{
	u8char_t **filenames;
	int *types;
	int *count;
};

typedef struct ncfselect {
	nclist_t * nclist;
	ncwin_t  * ncwin;
	char path[BUFSIZ];
	struct ncfselect_files files;
	bool focused;
	attr_t file_attr;
	attr_t dir_attr;
	attr_t link_attr;
	attr_t other_attr;
	CBRET (*callback)(void *, enum SCREEN, void *, chtype);
	void *userdata;
} ncfselect_t;

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
		);

void nc_fselect_set_path(ncfselect_t *s, const char *path);

void nc_fselect_set_focused(ncfselect_t *s, bool focused);
#define nc_set_focused_ncfselect(obj, focused) \
	nc_fselect_set_focused(obj, focused)

void
nc_fselect_activate(
		ncfselect_t *s, 
		void *userdata, 
		CBRET (*callback)(void *userdata, enum SCREEN type, void *object, chtype key)
		);
#define nc_activate_ncfselect(obj, d, cb) \
	nc_fselect_activate(obj, d, cb) 

void  nc_fselect_destroy(ncfselect_t *s);

#endif /* ifndef NC_FSELECT_H */
