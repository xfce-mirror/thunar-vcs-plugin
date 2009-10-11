
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
	tgh-branch.h							\
	tgh-branch.c							\
	tgh-clone.h							\
	tgh-clone.c							\
	tgh-common.h							\
	tgh-common.c							\
	tgh-reset.h							\
	tgh-reset.c							\
	tgh-status.h							\
	tgh-status.c							\
	tgh-dialog-common.h						\
	tgh-dialog-common.c						\
	tgh-branch-dialog.h						\
	tgh-branch-dialog.c						\
	tgh-file-selection-dialog.h					\
	tgh-file-selection-dialog.c					\
	tgh-reset-dialog.h						\
	tgh-reset-dialog.c						\
	tgh-status-dialog.h						\
	tgh-status-dialog.c						\
	tgh-transfer-dialog.h						\
	tgh-transfer-dialog.c

tvp_git_helper_CPPFLAGS =						\
	-DG_LOG_DOMAIN=\"tvp-git-helper\"				\
	$(AM_CPPFLAGS)

tvp_git_helper_CFLAGS =							\
	$(PLATFORM_CFLAGS)						\
	$(THUNAR_VFS_CFLAGS)						\
	$(GTK_CFLAGS)							\
	$(GLIB_CFLAGS)							\
	$(GOBJECT_CFLAGS)

tvp_git_helper_LDFLAGS =						\
	$(PLATFORM_LIBS)						\
	$(THUNAR_VFS_LIBS)						\
	$(GTK_LIBS)							\
	$(GLIB_LIBS)							\
	$(GOBJECT_LIBS)

# vi:set ts=8 sw=8 noet ai nocindent:
