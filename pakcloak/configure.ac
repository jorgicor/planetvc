#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_INIT([Pak Cloak],
	[1.00],
	[jorge.giner@hotmail.com],
	pakcloak,
	[http://jorgicor.sdfeu.org/pakcloak])

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR(config)
AM_INIT_AUTOMAKE([subdir-objects -Wall -Werror -Wportability gnu])
AC_CONFIG_HEADERS([config.h])

AC_PROG_CC
AC_PROG_CC_STDC

AX_CHECK_COMPILE_FLAG([-Wall], [WARN_CFLAGS="-Wall"], [], [])
AC_SUBST(WARN_CFLAGS)

AC_SUBST(ADD_CFLAGS)

AC_CONFIG_FILES([Makefile])

AC_OUTPUT
