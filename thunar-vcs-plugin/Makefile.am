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

AM_CPPFLAGS =								\
	-I$(top_builddir)						\
	-I$(top_srcdir)							\
	-DG_LOG_DOMAIN=\"thunar-vcs-plugin\"				\
	-DLIBEXECDIR=\"$(libexecdir)\"					\
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"				\
	$(PLATFORM_CPPFLAGS)

extensionsdir = $(libdir)/thunarx-3
extensions_LTLIBRARIES =						\
	thunar-vcs-plugin.la

thunar_vcs_plugin_la_SOURCES =						\
	tvp-provider.c							\
	tvp-provider.h							\
	thunar-vcs-plugin.c
if HAVE_SUBVERSION
thunar_vcs_plugin_la_SOURCES +=						\
	tvp-svn-backend.c						\
	tvp-svn-backend.h						\
	tvp-svn-action.c						\
	tvp-svn-action.h						\
	tvp-svn-property-page.c						\
	tvp-svn-property-page.h
endif
if HAVE_GIT
thunar_vcs_plugin_la_SOURCES +=						\
	tvp-git-action.c						\
	tvp-git-action.h
endif

thunar_vcs_plugin_la_CPPFLAGS =						\
	$(AM_CPPFLAGS)
if HAVE_SUBVERSION
thunar_vcs_plugin_la_CPPFLAGS +=					\
	-DTVP_SVN_HELPER=\"$(libexecdir)/tvp-svn-helper\"
endif
if HAVE_GIT
thunar_vcs_plugin_la_CPPFLAGS +=					\
	-DTVP_GIT_HELPER=\"$(libexecdir)/tvp-git-helper\"
endif

thunar_vcs_plugin_la_CFLAGS =						\
	$(PLATFORM_CFLAGS)						\
	$(GTK_CFLAGS)							\
	$(GLIB_CFLAGS)							\
	$(GOBJECT_CFLAGS)						\
	$(THUNARX_CFLAGS)						\
	$(LIBXFCE4UTIL_CFLAGS)
if HAVE_SUBVERSION
thunar_vcs_plugin_la_CFLAGS +=						\
	$(APR_CFLAGS)
endif

thunar_vcs_plugin_la_LDFLAGS =						\
	-avoid-version							\
	-export-dynamic							\
	-module								\
	$(PLATFORM_LDFLAGS)						\
	$(GTK_LIBS)							\
	$(GLIB_LIBS)							\
	$(GOBJECT_LIBS)							\
	$(THUNARX_LIBS)							\
	$(LIBXFCE4UTIL_LIBS)
if HAVE_SUBVERSION
thunar_vcs_plugin_la_LDFLAGS +=						\
	$(APR_LIBS)
endif

EXTRA_DIST =								\
	meson.build

# vi:set ts=8 sw=8 noet ai nocindent:
