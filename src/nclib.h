/**
 * File              : nclib.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 12.06.2023
 * Last Modified Date: 26.06.2023
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#ifndef NC_LIB_H
#define NC_LIB_H

#include <curses.h>
#include <panel.h>

#include "colors.h"
#include "ncwin.h"
#include "nlist.h"
#include "nclabel.h"

void nc_init(
		const char *locale,
		int color
		);

#endif /* ifndef NC_LIB_H */
