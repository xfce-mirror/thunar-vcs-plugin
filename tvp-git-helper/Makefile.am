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
        -DBINDIR=\"$(bindir)\"                                          \
        -DDATADIR=\"$(datadir)\"                                        \
	-DLIBDIR=\"$(libdir)\"						\
	-DLIBEXECDIR=\"$(libexecdir)\"					\
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"				\
	$(PLATFORM_CPPFLAGS)

libexec_PROGRAMS =							\
	tvp-git-helper

tvp_git_helper_SOURCES =						\
	main.c								\
	tgh-add.h							\
	tgh-add.c							\
	tgh-blame.h							\
	tgh-blame.c							\
	tgh-branch.h							\
	tgh-branch.c							\
	tgh-clean.h							\
	tgh-clean.c							\
	tgh-clone.h							\
	tgh-clone.c							\
	tgh-common.h							\
	tgh-common.c							\
	tgh-log.h							\
	tgh-log.c							\
	tgh-move.h							\
	tgh-move.c							\
	tgh-reset.h							\
	tgh-reset.c							\
	tgh-stash.h							\
	tgh-stash.c							\
	tgh-status.h							\
	tgh-status.c							\
	tgh-dialog-common.h						\
	tgh-dialog-common.c						\
	tgh-blame-dialog.h						\
	tgh-blame-dialog.c						\
	tgh-branch-dialog.h						\
	tgh-branch-dialog.c						\
	tgh-clean-dialog.h						\
	tgh-clean-dialog.c						\
	tgh-file-selection-dialog.h					\
	tgh-file-selection-dialog.c					\
	tgh-log-dialog.h						\
	tgh-log-dialog.c						\
	tgh-notify-dialog.h						\
	tgh-notify-dialog.c						\
	tgh-stash-dialog.h						\
	tgh-stash-dialog.c						\
	tgh-status-dialog.h						\
	tgh-status-dialog.c						\
	tgh-transfer-dialog.h						\
	tgh-transfer-dialog.c						\
	tgh-cell-renderer-graph.h					\
	tgh-cell-renderer-graph.c

tvp_git_helper_CPPFLAGS =						\
	-DG_LOG_DOMAIN=\"tvp-git-helper\"				\
	$(AM_CPPFLAGS)

tvp_git_helper_CFLAGS =					\
	$(PLATFORM_CFLAGS)					\
	$(LIBXFCE4UI_CFLAGS)				\
	$(LIBXFCE4UTIL_CFLAGS)				\
	$(GTK_CFLAGS)						\
	$(GLIB_CFLAGS)						\
	$(GOBJECT_CFLAGS)					\
	$(EXO_CFLAGS)

tvp_git_helper_LDFLAGS =				\
	$(PLATFORM_LDFLAGS)					\
	$(LIBXFCE4UI_LIBS)				\
	$(LIBXFCE4UTIL_LIBS)				\
	$(GTK_LIBS)							\
	$(GLIB_LIBS)						\
	$(GOBJECT_LIBS)						\
	$(EXO_LIBS)

EXTRA_DIST =								\
	meson.build

# vi:set ts=8 sw=8 noet ai nocindent:
