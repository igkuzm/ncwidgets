/**
 * File              : utils.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 12.06.2023
 * Last Modified Date: 27.06.2023
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#ifndef NC_UTILS_H
#define NC_UTILS_H

#include <curses.h>
#include <ncurses.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

static void
_buf2attr(attr_t *attr, char *str, int add)
{
	char buf[32];
	int i = 0, l = 0;
	while (str[i]) {
		char c = str[i++];
		switch (c) {
			case 'B': 
				if (add)
					*attr |= A_BOLD;
				else
					*attr &= ~A_BOLD;
				break;
			case 'U':	
				if (add)
					*attr |= A_UNDERLINE;
				else
					*attr &= ~A_UNDERLINE;
				break;

			case 0: case 1: case 2: case 3: case 4:
			case 5: case 6: case 7: case 8: case 9:
				buf[l++] = c;
				break;

			default:
				break;
		}
	}
	buf[l++] = 0;
	
	int color = atoi(buf);
	if (color){
		if (add)
			*attr |= COLOR_PAIR(color);
		else
			*attr &= ~COLOR_PAIR(color);
	}
}

static chtype  *
str2chtypestr(const char *str, int color)
{
	size_t len = strlen(str);
	chtype *chtypestr = malloc(len * sizeof(chtype));
	if (!chtypestr)
		return NULL;

	size_t i, l = 0; 
	attr_t attr = A_NORMAL|COLOR_PAIR(color);
	char buf[32];
	int buflen = 0;
	for (i = 0; i < len; ++i) {
		if (str[i] == '<'){
			if (str[i+1] == '/' || str[i+1] == '!'){
				// start of attributes
				int add = 1;
				if (str[i+1] == '!')
					add = 0;
				i+=2;
				while (str[i] != '>') {
					buf[buflen++] = str[i++];
				}
				buf[buflen++] = 0;
				_buf2attr(&attr, buf, add);
				i++;
				buflen = 0;
			}
		}

		chtype c = (int)str[i] | attr;
		chtypestr[l++] = c;
	}
	chtypestr[l++] = 0;
	return chtypestr;
}

typedef struct {
	char utf8[7];
	attr_t attr;
} u8char_t;

static char *
ucharstr2str(const u8char_t *ucharstr)
{
	int size = BUFSIZ;
	char *str = malloc(size);
	if (!str)
		return NULL;

	size_t i = 0, l = 0;
	while (ucharstr[i].utf8[0]){
		const char *s = ucharstr[i++].utf8;
		int len = strlen(s);

		//realloc size
		if (size < l + len){
			size += BUFSIZ;
			void *ptr = realloc(str, size);
			if (!ptr)
				return NULL;
			str = ptr;
		}

		//set string
		int k;
		for (k = 0; k < len; ++k) {
			str[l++] = s[k];
		}
	}

	//terminate string
	str[l] = 0;

	return str;
}

static u8char_t *
str2ucharstr(const char *str, int color)
{
	size_t len = strlen(str);
	u8char_t *ucharstr = malloc(len * sizeof(u8char_t));
	if (!ucharstr)
		return NULL;

	size_t i, l = 0, k; 
	attr_t attr = A_NORMAL|COLOR_PAIR(color);
	char buf[32];
	int buflen = 0;
	for (i = 0; i < len; ++i) {
		if (str[i] == '<'){
			if (str[i+1] == '/' || str[i+1] == '!'){
				// start of attributes
				int add = 1;
				if (str[i+1] == '!')
					add = 0;
				i+=2;
				while (str[i] != '>') {
					buf[buflen++] = str[i++];
				}
				buf[buflen++] = 0;
				_buf2attr(&attr, buf, add);
				i++;
				buflen = 0;
			}
		}

		unsigned char c = str[i];
		if (c >= 252){/* 6-bytes */
			ucharstr[l].utf8[0] = str[i++]; 
			ucharstr[l].utf8[1] = str[i++]; 
			ucharstr[l].utf8[2] = str[i++]; 
			ucharstr[l].utf8[3] = str[i++]; 
			ucharstr[l].utf8[4] = str[i++]; 
			ucharstr[l].utf8[5] = str[i]; 
			ucharstr[l].utf8[6] = 0; 
		} 
		else if (c >= 248){/* 5-bytes */
			ucharstr[l].utf8[0] = str[i++]; 
			ucharstr[l].utf8[1] = str[i++]; 
			ucharstr[l].utf8[2] = str[i++]; 
			ucharstr[l].utf8[3] = str[i++]; 
			ucharstr[l].utf8[4] = str[i]; 
			ucharstr[l].utf8[5] = 0; 
		}
		else if (c >= 240){/* 4-bytes */
			ucharstr[l].utf8[0] = str[i++]; 
			ucharstr[l].utf8[1] = str[i++]; 
			ucharstr[l].utf8[2] = str[i++]; 
			ucharstr[l].utf8[3] = str[i]; 
			ucharstr[l].utf8[4] = 0; 
		} 
		else if (c >= 224){/* 3-bytes */
			ucharstr[l].utf8[0] = str[i++]; 
			ucharstr[l].utf8[1] = str[i++]; 
			ucharstr[l].utf8[2] = str[i]; 
			ucharstr[l].utf8[3] = 0; 
		}
		else if (c >= 192){/* 2-bytes */
			ucharstr[l].utf8[0] = str[i++]; 
			ucharstr[l].utf8[1] = str[i]; 
			ucharstr[l].utf8[2] = 0; 
		} 
		else{/* 1-byte */
			ucharstr[l].utf8[0] = str[i]; 
			ucharstr[l].utf8[1] = 0; 
		} 
		
		ucharstr[l++].attr = attr;
	}
	ucharstr[l++].utf8[0] = 0;
	return ucharstr;
}

static char *
move_char_right(char *str)
{
	int i = 0;
	unsigned char c = str[i];
	if      (c >= 252) /* 6-bytes */
		i+=6;
	else if (c >= 248) /* 5-bytes */
		i+=5;
	else if (c >= 240) /* 4-bytes */
		i+=4;
	else if (c >= 224) /* 3-bytes */
		i+=3;
	else if (c >= 192) /* 2-bytes */
		i+=2;
	else               /* 1-byte  */
		i++;
		
	return &str[i];
}

static int
strchars(char *str)
{
	int c = 0;
	while (*str){
		str = move_char_right(str);
		c++;
	}
	
	return c;	
}

static size_t
ucharstrlen(u8char_t *str)
{
	size_t i = 0;
	while (str[i].utf8[0])
		i++;
	
	return i;	
}

static void 
reverse_chtypestr(chtype **chtypestr){
	int i = 0;
	while (chtypestr[0][i]){
		chtypestr[0][i] = chtypestr[0][i] | A_REVERSE;
		i++;	
	}
}

#endif /* ifndef NC_UTILS_H */
