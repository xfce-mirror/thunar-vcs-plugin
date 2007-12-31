
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
	tsp-svn-helper

tsp_svn_helper_SOURCES =						\
	main.c								\
	tsh-common.h							\
	tsh-common.c							\
	tsh-add.h							\
	tsh-add.c							\
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
	tsh-move.h							\
	tsh-move.c							\
	tsh-resolved.h							\
	tsh-resolved.c							\
	tsh-revert.h							\
	tsh-revert.c							\
	tsh-status.h							\
	tsh-status.c							\
	tsh-unlock.h							\
	tsh-unlock.c							\
	tsh-update.h							\
	tsh-update.c							\
	gtkfilechooserentry.h						\
	gtkfilechooserentry.c						\
	gtkfilechooserutils.h						\
	gtkfilechooserutils.c						\
	gtkfilechooser.h						\
	gtkfilechooser.c						\
	gtkfilechooserdefault.c						\
	gtkprivate.h							\
	gtkintl.h							\
	tsh-dialog-common.h						\
	tsh-dialog-common.c						\
	tsh-login-dialog.h						\
	tsh-login-dialog.c						\
	tsh-file-dialog.h						\
	tsh-file-dialog.c						\
	tsh-trust-dialog.h						\
	tsh-trust-dialog.c						\
	tsh-notify-dialog.h						\
	tsh-notify-dialog.c						\
	tsh-status-dialog.h						\
	tsh-status-dialog.c						\
	tsh-lock-dialog.h						\
	tsh-lock-dialog.c						\
	tsh-log-message-dialog.h					\
	tsh-log-message-dialog.c					\
	tsh-transfer-dialog.h						\
	tsh-transfer-dialog.c

tsp_svn_helper_CPPFLAGS =						\
	-DG_LOG_DOMAIN=\"tsp-svn-helper\"

tsp_svn_helper_CFLAGS =							\
	$(PLATFORM_CFLAGS)						\
	$(THUNAR_VFS_CFLAGS)						\
	$(GTK_CFLAGS)							\
	$(GLIB_CFLAGS)							\
	$(GOBJECT_CFLAGS)						\
	$(APR_CFLAGS)

tsp_svn_helper_LDFLAGS =						\
	$(PLATFORM_LIBS)						\
	$(THUNAR_VFS_LIBS)						\
	$(GTK_LIBS)							\
	$(GLIB_LIBS)							\
	$(GOBJECT_LIBS)							\
	$(APR_LIBS)

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
