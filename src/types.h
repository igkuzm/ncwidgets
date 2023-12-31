/**
 * File              : types.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 14.06.2023
 * Last Modified Date: 30.06.2023
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#ifndef NC_TYPES_H
#define NC_TYPES_H

#define NCSCREENS\
	NCSCREEN(nclist)\
	NCSCREEN(nclabel)\
	NCSCREEN(ncbutton)\
	NCSCREEN(ncentry)\
	NCSCREEN(nccalendar)\
	NCSCREEN(ncselection)\

enum SCREEN {
#define NCSCREEN(title) SCREEN_##title,
	NCSCREENS
#undef NCSCREEN
};

typedef enum {
	CBNONE = 0,
	CBBREAK,
	CBCONTINUE,
} CBRET;

#endif /* ifndef NC_TYPES_H */
