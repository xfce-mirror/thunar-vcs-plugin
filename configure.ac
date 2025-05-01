#-
# Copyright (C) 2007-2011  Peter de Ridder <peter@xfce.org>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

dnl ***************************
dnl *** Version information ***
dnl ***************************
XDT_VERSION_INIT([0.3.0], [git])
m4_define([copyright_year], [2025])

dnl ***************************
dnl *** Initialize autoconf ***
dnl ***************************
AC_COPYRIGHT([Copyright (c) 2006-copyright_year() The Xfce development team. All rights reserved.])
AC_INIT([thunar-vcs-plugin], [xdt_version], [https://gitlab.xfce.org/thunar-plugins/thunar-vcs-plugin])
AC_REVISION([xdt_version_build])
AC_PREFIX_DEFAULT([$(pkg-config --variable prefix thunarx-3 || echo /usr/local)])
AC_PREREQ([2.69])
AC_CANONICAL_TARGET()
AC_DEFINE([VERSION_FULL], [PACKAGE_VERSION], [Alias for VERSION and PACKAGE_VERSION for meson compatibility])
COPYRIGHT_YEAR=copyright_year()
AC_DEFINE_UNQUOTED([COPYRIGHT_YEAR], ["$COPYRIGHT_YEAR"], [Copyright year])
AC_SUBST([COPYRIGHT_YEAR])

dnl ***************************
dnl *** Initialize automake ***
dnl ***************************
AM_INIT_AUTOMAKE([1.8 dist-bzip2 tar-ustar foreign])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIRS([m4])
AM_MAINTAINER_MODE()
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

dnl ********************************
dnl *** Check for common headers ***
dnl ********************************
AC_CHECK_HEADERS([sys/wait.h])

dnl ********************************
dnl *** Check for basic programs ***
dnl ********************************
AC_PROG_CC()
AC_PROG_INSTALL()
AM_PROG_CC_C_O()

dnl **************************
dnl *** Initialize libtool ***
dnl **************************
LT_PREREQ([2.2.6])
LT_INIT([disable-static])

dnl ******************************
dnl *** Check for i18n support ***
dnl ******************************
GETTEXT_PACKAGE="$PACKAGE"
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], ["$GETTEXT_PACKAGE"], [Name of default gettext domain])
AC_SUBST([GETTEXT_PACKAGE])
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.19.8])
AC_SEARCH_LIBS([bind_textdomain_codeset], [intl],
  [AC_DEFINE([HAVE_BIND_TEXTDOMAIN_CODESET], [1], [Define to 1 if you have the 'bind_textdomain_codeset' function.])],
  [])

dnl ***********************************
dnl *** Check for required packages ***
dnl ***********************************
XDT_CHECK_PACKAGE([THUNARX], [thunarx-3], [4.18.0])
XDT_CHECK_PACKAGE([EXO], [exo-2], [4.18.0])
XDT_CHECK_PACKAGE([LIBXFCE4UI], [libxfce4ui-2], [4.18.0])
XDT_CHECK_PACKAGE([LIBXFCE4UTIL], [libxfce4util-1.0], [4.18.0])
XDT_CHECK_PACKAGE([GTK], [gtk+-3.0], [3.20.0])
XDT_CHECK_PACKAGE([GLIB], [glib-2.0], [2.32.0])
XDT_CHECK_PACKAGE([GTHREAD], [gthread-2.0], [2.32.0])
XDT_CHECK_PACKAGE([GOBJECT], [gobject-2.0], [2.32.0])

dnl ***********************************
dnl *** Check for optional packages ***
dnl ***********************************
XDT_CHECK_OPTIONAL_PACKAGE([APR], [apr-1], [0.9.7], [subversion], [libsvn])
XDT_CHECK_OPTIONAL_PACKAGE([APRUTIL], [apr-util-1], [0.9.1], [subversion], [libsvn])
if test x"$APR_FOUND$APRUTIL_FOUND" = x"yesyes"; then
    found_subversion=yes
    AC_CHECK_LIB([svn_subr-1], [main], , [found_subversion=no])
    AC_CHECK_LIB([svn_fs-1], [main], , [found_subversion=no])
    AC_CHECK_LIB([svn_wc-1], [main], , [found_subversion=no])
    AC_CHECK_LIB([svn_client-1], [main], , [found_subversion=no])
    if test x"$found_subversion" = x"yes"; then
        AC_DEFINE([HAVE_SUBVERSION], [1], [Define if libsvn is present])
    fi
else
    AC_MSG_CHECKING([for optional package subversion])
    AC_MSG_RESULT([disabled])
fi
AM_CONDITIONAL([HAVE_SUBVERSION], [test x"$found_subversion" = x"yes"])

AC_ARG_ENABLE([git], 
    AS_HELP_STRING([--enable-git], [Enable git (default)])
AS_HELP_STRING([--disable-git], [Disable git]),
    [check_for_git=$enableval], [check_for_git=yes])
AC_MSG_CHECKING([for optional package git])
if test x"$check_for_git" = x"yes"; then
    found_git=yes
    AC_MSG_RESULT([enabled])
    AC_DEFINE([HAVE_GIT], [1], [Define if git is endabled])
else
    AC_MSG_RESULT([disabled])
fi
AM_CONDITIONAL([HAVE_GIT], [test x"$found_git" = x"yes"])

dnl ***********************************
dnl *** Check for debugging support ***
dnl ***********************************
XDT_FEATURE_DEBUG([xdt_debug_default])

dnl **************************************
dnl *** Check for linker optimizations ***
dnl **************************************
XDT_FEATURE_LINKER_OPTS()

dnl *********************************
dnl *** Substitute platform flags ***
dnl *********************************
AC_MSG_CHECKING([PLATFORM_CPPFLAGS])
AC_MSG_RESULT([$PLATFORM_CPPFLAGS])
AC_SUBST([PLATFORM_CPPFLAGS])
AC_MSG_CHECKING([PLATFORM_CFLAGS])
AC_MSG_RESULT([$PLATFORM_CFLAGS])
AC_SUBST([PLATFORM_CFLAGS])
AC_MSG_CHECKING([PLATFORM_LDFLAGS])
AC_MSG_RESULT([$PLATFORM_LDFLAGS])
AC_SUBST([PLATFORM_LDFLAGS])

AC_CONFIG_FILES([
Makefile
icons/Makefile
icons/16x16/Makefile
icons/24x24/Makefile
icons/48x48/Makefile
po/Makefile.in
thunar-vcs-plugin/Makefile
tvp-svn-helper/Makefile
tvp-git-helper/Makefile
])
AC_OUTPUT()

dnl ***************************
dnl *** Print configuration ***
dnl ***************************
echo
echo "Build Configuration:"
echo
dnl echo "* Installation prefix:                $prefix"
if test x"$found_subversion" = x"yes"; then
echo "* Subversion support:                 yes"
else
echo "* Subversion support:                 no"
fi
if test x"$found_git" = x"yes"; then
echo "* Git support:                        yes"
else
echo "* Git support:                        no"
fi
if test x"$found_subversion" != x"yes" -a x"$found_git" != x"yes"; then
echo
echo "WARNING: Both subversion and git are disabled. This is probably not intended."
fi
echo
