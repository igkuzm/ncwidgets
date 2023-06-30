noinst_LIBRARIES = libncwidgets.a

libncwidgets_a_SOURCES = \
		colors.h keys.h utils.h \
		ncbutton.h    ncbutton.c \
		nccalendar.h  nccalendar.c \
		ncentry.h     ncentry.c \
		nclabel.h     nclabel.c \
		nclist.h      nclist.c \
		ncscreen.h    ncscreen.c \
		ncselection.h ncselection.c \
		ncwidgets.h   ncwidgets.c \
		ncwin.h       ncwin.c

libncwidgets_a_CFLAGS = @CURSES_CFLAGS@
libncwidgets_a_LIBADD = @PANEL_LIBS@ @CURSES_LIBS@ @CURSES_CFLAGS@
