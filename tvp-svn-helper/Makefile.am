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
	tvp-svn-helper

tvp_svn_helper_SOURCES =						\
	main.c								\
	tsh-common.h							\
	tsh-common.c							\
	tsh-add.h							\
	tsh-add.c							\
	tsh-blame.h							\
	tsh-blame.c							\
	tsh-checkout.h							\
	tsh-checkout.c							\
	tsh-cleanup.h							\
	tsh-cleanup.c							\
	tsh-commit.h							\
	tsh-commit.c							\
	tsh-copy.h							\
	tsh-copy.c							\
	tsh-delete.h							\
	tsh-delete.c							\
	tsh-export.h							\
	tsh-export.c							\
	tsh-import.h							\
	tsh-import.c							\
	tsh-lock.h							\
	tsh-lock.c							\
	tsh-log.h							\
	tsh-log.c							\
	tsh-move.h							\
	tsh-move.c							\
	tsh-properties.h						\
	tsh-properties.c						\
	tsh-relocate.h							\
	tsh-relocate.c							\
	tsh-resolved.h							\
	tsh-resolved.c							\
	tsh-revert.h							\
	tsh-revert.c							\
	tsh-status.h							\
	tsh-status.c							\
	tsh-switch.h							\
	tsh-switch.c							\
	tsh-unlock.h							\
	tsh-unlock.c							\
	tsh-update.h							\
	tsh-update.c							\
	tsh-tree-common.h						\
	tsh-tree-common.c						\
	tsh-dialog-common.h						\
	tsh-dialog-common.c						\
	tsh-blame-dialog.h						\
	tsh-blame-dialog.c						\
	tsh-file-dialog.h						\
	tsh-file-dialog.c						\
	tsh-file-selection-dialog.h					\
	tsh-file-selection-dialog.c					\
	tsh-lock-dialog.h						\
	tsh-lock-dialog.c						\
	tsh-log-dialog.h						\
	tsh-log-dialog.c						\
	tsh-log-message-dialog.h					\
	tsh-log-message-dialog.c					\
	tsh-login-dialog.h						\
	tsh-login-dialog.c						\
	tsh-notify-dialog.h						\
	tsh-notify-dialog.c						\
	tsh-properties-dialog.h						\
	tsh-properties-dialog.c						\
	tsh-relocate-dialog.h						\
	tsh-relocate-dialog.c						\
	tsh-status-dialog.h						\
	tsh-status-dialog.c						\
	tsh-transfer-dialog.h						\
	tsh-transfer-dialog.c						\
	tsh-trust-dialog.h						\
	tsh-trust-dialog.c

tvp_svn_helper_CPPFLAGS =						\
	-DG_LOG_DOMAIN=\"tvp-svn-helper\"				\
	$(AM_CPPFLAGS)

tvp_svn_helper_CFLAGS =							\
	$(PLATFORM_CFLAGS)						\
	$(LIBXFCE4UTIL_CFLAGS)						\
	$(GTK_CFLAGS)							\
	$(GLIB_CFLAGS)							\
	$(GTHREAD_CFLAGS)						\
	$(GOBJECT_CFLAGS)						\
	$(APR_CFLAGS)

tvp_svn_helper_LDFLAGS =						\
	$(PLATFORM_LDFLAGS)						\
	$(LIBXFCE4UTIL_LIBS)						\
	$(GTK_LIBS)							\
	$(GLIB_LIBS)							\
	$(GTHREAD_LIBS)							\
	$(GOBJECT_LIBS)							\
	$(APR_LIBS)

# vi:set ts=8 sw=8 noet ai nocindent:
