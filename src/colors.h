/**
 * File              : colors.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 14.05.2023
 * Last Modified Date: 13.06.2023
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */


#ifndef COLORS_H
#define COLORS_H

#include <curses.h>

#define _COLOR_STR(x, y) "</" #x ">" y "<!" #x ">"
#define COLOR_STR(x, y) _COLOR_STR(x, y)

#define WHITE_ON_WHITE      1
#define WHITE_ON_RED        2
#define WHITE_ON_GREEN      3
#define WHITE_ON_YELLOW     4
#define WHITE_ON_BLUE       5
#define WHITE_ON_MAGENTA    6
#define WHITE_ON_CYAN       7
#define WHITE_ON_BLACK      8

#define RED_ON_WHITE        9
#define RED_ON_RED          10
#define RED_ON_GREEN        11
#define RED_ON_YELLOW       12
#define RED_ON_BLUE         13
#define RED_ON_MAGENTA      14
#define RED_ON_CYAN         15
#define RED_ON_BLACK        16

#define GREEN_ON_WHITE      17
#define GREEN_ON_RED        18
#define GREEN_ON_GREEN      19
#define GREEN_ON_YELLOW     20
#define GREEN_ON_BLUE       21
#define GREEN_ON_MAGENTA    22
#define GREEN_ON_CYAN       23
#define GREEN_ON_BLACK      24

#define YELLOW_ON_WHITE     25
#define YELLOW_ON_RED       26
#define YELLOW_ON_GREEN     27
#define YELLOW_ON_YELLOW    28
#define YELLOW_ON_BLUE      29
#define YELLOW_ON_MAGENTA   30
#define YELLOW_ON_CYAN      31
#define YELLOW_ON_BLACK     32

#define BLUE_ON_WHITE       33
#define BLUE_ON_RED         34
#define BLUE_ON_GREEN       35
#define BLUE_ON_YELLOW      36
#define BLUE_ON_BLUE        37
#define BLUE_ON_MAGENTA     38
#define BLUE_ON_CYAN        39
#define BLUE_ON_BLACK       40

#define MAGENTA_ON_WHITE    41
#define MAGENTA_ON_RED      42
#define MAGENTA_ON_GREEN    43
#define MAGENTA_ON_YELLOW   44
#define MAGENTA_ON_BLUE     45
#define MAGENTA_ON_MAGENTA  46
#define MAGENTA_ON_CYAN     47
#define MAGENTA_ON_BLACK    48

#define CYAN_ON_WHITE       49
#define CYAN_ON_RED         50
#define CYAN_ON_GREEN       51
#define CYAN_ON_YELLOW      52
#define CYAN_ON_BLUE        53
#define CYAN_ON_MAGENTA     54
#define CYAN_ON_CYAN        55
#define CYAN_ON_BLACK       56
	
#define BLACK_ON_WHITE      57
#define BLACK_ON_RED        58
#define BLACK_ON_GREEN      59
#define BLACK_ON_YELLOW     60
#define BLACK_ON_BLUE       61
#define BLACK_ON_MAGENTA    62
#define BLACK_ON_CYAN       63
#define BLACK_ON_BLACK      64

static short ALLCOLORS[] = 
{
	COLOR_WHITE,
	COLOR_RED,
	COLOR_GREEN,
	COLOR_YELLOW,
	COLOR_BLUE,
	COLOR_MAGENTA,
	COLOR_CYAN,
	COLOR_BLACK
};

static void
init_colors()
{
	short i, k, color = 1;
	for (i = 0; i < 8; ++i)
		for (k = 0; k < 8; ++k)
			init_pair(color++, ALLCOLORS[i], ALLCOLORS[k]);
}

#endif /* ifndef COLORS_H */			
