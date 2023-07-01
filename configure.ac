AC_INIT(ncwidgets, 1.0)
AM_INIT_AUTOMAKE

AC_PROG_CC
AC_PROG_RANLIB
AC_CONFIG_MACRO_DIR([m4])

AX_WITH_CURSES
AX_WITH_CURSES_PANEL
if test "x$ax_cv_ncursesw" != xyes && test "x$ax_cv_ncurses" && test "x$ax_cv_curses" != xyes; then
	AC_MSG_ERROR([requires Curses or NcursesW or Ncurses library])
fi

AC_CONFIG_FILES([
Makefile
src/Makefile
])

AC_OUTPUT
