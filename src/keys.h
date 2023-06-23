/**
 * File              : keys.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 13.06.2023
 * Last Modified Date: 21.06.2023
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#ifndef NC_KEYS_H
#define NC_KEYS_H

#undef KEY_ESC
#define KEY_ESC 27

#undef CTRL
#define CTRL(c) ((c) & 0x1f)

#undef  KEY_TAB
#define KEY_TAB		'\t'	/* Tab key.				*/

#undef  KEY_RETURN
#define KEY_RETURN	'\012'	/* Return key				*/

#undef  KEY_SPACE
#define KEY_SPACE	' '	/* Space key				*/

#undef  KEY_DELETE
#define KEY_DELETE	'\177'	/* Delete key				*/

#ifdef __WIN32__
	#define MOUSE_LEFT_BUTTON  NCURSES_MOUSE_MASK(1, NCURSES_BUTTON_PRESSED)
	#define MOUSE_RIGHT_BUTTON NCURSES_MOUSE_MASK(2, NCURSES_BUTTON_PRESSED)
	#define MOUSE_SCROLL_DOWN  NCURSES_MOUSE_MASK(4, NCURSES_BUTTON_PRESSED)
	#define MOUSE_SCROLL_UP    NCURSES_MOUSE_MASK(5, NCURSES_BUTTON_PRESSED)
#elif defined __APPLE__
	#define MOUSE_LEFT_BUTTON  NCURSES_MOUSE_MASK(1, NCURSES_BUTTON_PRESSED)
	#define MOUSE_RIGHT_BUTTON NCURSES_MOUSE_MASK(3, NCURSES_BUTTON_PRESSED)
	#define MOUSE_SCROLL_DOWN  NCURSES_MOUSE_MASK(2, NCURSES_BUTTON_PRESSED)
	#define MOUSE_SCROLL_UP    NCURSES_MOUSE_MASK(4, NCURSES_BUTTON_PRESSED)
#else
	#define MOUSE_LEFT_BUTTON  NCURSES_MOUSE_MASK(1, NCURSES_BUTTON_PRESSED)
	#define MOUSE_RIGHT_BUTTON NCURSES_MOUSE_MASK(2, NCURSES_BUTTON_PRESSED)
	#define MOUSE_SCROLL_DOWN  NCURSES_MOUSE_MASK(4, NCURSES_BUTTON_PRESSED)
	#define MOUSE_SCROLL_UP    NCURSES_MOUSE_MASK(5, NCURSES_BUTTON_PRESSED)
#endif

#endif /* ifndef NC_KEYS_H */
