AC_INIT([Fvwm95], [Forever], [dave@flaterco.com])
AC_CONFIG_SRCDIR([Makefile.am])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([dist-xz no-dist-gzip subdir-objects -Wall -Werror])
AC_LANG([C])
AC_PROG_CC
# Was: AC_PROG_CC([gcc])
AC_REQUIRE_CPP
m4_version_prereq([2.70], [AM_PROG_LEX(noyywrap)], [AM_PROG_LEX])
AC_PROG_YACC
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
AC_PROG_RANLIB

# adl_RECURSIVE_EVAL:  patch for badly needed, missing autoconf feature.
# Copied 2016-01-03 from http://ac-archive.sourceforge.net/adl/relpaths.html
# License: GPLWithACException
#   This program is free software; you can redistribute it and/or modify it
#   under the terms of the GNU General Public License as published by the
#   Free Software Foundation. As a special exception, the respective Autoconf
#   Macro's copyright owner gives unlimited permission to copy, distribute
#   and modify the configure scripts that are the output of Autoconf when
#   processing the Macro. You need not follow the terms of the GNU General
#   Public License when using or distributing such scripts
# Author: Alexandre Duret-Lutz <duret_g@epita.fr>
# Version 2001-05-25
dnl adl_RECURSIVE_EVAL(VALUE, RESULT)
dnl =================================
dnl Interpolate the VALUE in loop until it doesn't change,
dnl and set the result to $RESULT.
dnl WARNING: It's easy to get an infinite loop with some unsane input.
AC_DEFUN([adl_RECURSIVE_EVAL],
[_lcl_receval="$1"
$2=`(test "x$prefix" = xNONE && prefix="$ac_default_prefix"
     test "x$exec_prefix" = xNONE && exec_prefix="${prefix}"
     _lcl_receval_old=''
     while test "[$]_lcl_receval_old" != "[$]_lcl_receval"; do
       _lcl_receval_old="[$]_lcl_receval"
       eval _lcl_receval="\"[$]_lcl_receval\""
     done
     echo "[$]_lcl_receval")`])

# Warning: must synchronize with path setup in Makefile.am / makemakefileam.sh
# and again in sample.fvwmrc/system.fvwm95rc.in.
# In configure.ac, we have only the variables mentioned by configure --help.
# The rc_ variables are for sample.fvwmrc/system.fvwm95rc.in.  This might be
# made a little cleaner by passing rc_icondir etc. instead of just
# rc_pkgdatadir and rc_pkglibexecdir, but supposedly this entire approach is
# wrong--it's supposed to be done by make instead.
adl_RECURSIVE_EVAL(${sysconfdir},SYSCONFDIR)
adl_RECURSIVE_EVAL(${libexecdir},LIBEXECDIR)
adl_RECURSIVE_EVAL(${datadir},DATADIR)
AC_DEFINE_UNQUOTED([FVWMDIR], ["${SYSCONFDIR}"], [Where to look for system.fvwm95rc.])
# pkglibexecdir
AC_DEFINE_UNQUOTED([FVWM_MODULEDIR], ["${LIBEXECDIR}/${PACKAGE}"], [Where to look for modules.])
AC_SUBST([rc_pkglibexecdir], ["${LIBEXECDIR}/${PACKAGE}"])
# pkgdatadir
AC_SUBST([rc_pkgdatadir], ["${DATADIR}/${PACKAGE}"])
# iconsdir
AC_DEFINE_UNQUOTED([FVWM_ICONDIR], ["/usr/include/X11/bitmaps:/usr/share/pixmaps:${DATADIR}/${PACKAGE}/mini-icons:${DATADIR}/${PACKAGE}/pixmaps"], [Where to look for icons.])
# pluginsdir
AC_DEFINE_UNQUOTED([PLUGINS], ["${LIBEXECDIR}/${PACKAGE}/plugins"], [Where to look for FvwmTaskBar plugins ("goodies").])
# scriptsdir
AC_DEFINE_UNQUOTED([FVWM_SCRIPTS], ["${DATADIR}/${PACKAGE}/scripts"], [Where to look for FvwmScript scripts.])

# Defuse ifdefs for things that used to be optional.
AC_DEFINE([XPM], [1], [Have this.])
AC_DEFINE([SHAPE], [1], [Have this.])
AC_DEFINE([HAVE_GETHOSTNAME], [1], [Have this.])
AC_DEFINE([HAVE_UNAME], [1], [Have this.])
AC_DEFINE([HAVE_SYSCONF], [1], [Have this.])
AC_DEFINE([HAVE_WAITPID], [1], [Have this.])
AC_DEFINE([HAVE_READLINE], [1], [Have this.])
AC_DEFINE([HAVE_STRERROR], [1], [Have this.])
AC_DEFINE([HAVE_STRCASECMP], [1], [Have this.])
AC_DEFINE([HAVE_STRNCASECMP], [1], [Have this.])

AC_CONFIG_HEADERS([include/FVWMconfig.h])
AC_CONFIG_FILES([Makefile sample.fvwmrc/system.fvwm95rc])
AC_OUTPUT
