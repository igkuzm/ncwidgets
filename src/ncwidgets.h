/**
 * File              : ncwidgets.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 12.06.2023
 * Last Modified Date: 30.06.2023
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#ifndef NC_WIDGETS_H
#define NC_WIDGETS_H

#include <curses.h>
#include <panel.h>

#include "colors.h"
#include "ncscreen.h"

void nc_init(
		const char *locale,
		int color
		);

void nc_quit();

#endif /* ifndef NC_WIDGETS_H */
