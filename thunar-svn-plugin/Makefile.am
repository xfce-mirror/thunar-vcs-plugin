
INCLUDES =								\
	-I$(top_builddir)						\
	-I$(top_srcdir)							\
	-DG_LOG_DOMAIN=\"thunar-svn-plugin\"				\
	-DEXO_API_SUBJECT_TO_CHANGE					\
	-DLIBEXECDIR=\"$(libexecdir)\"					\
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"				\
	$(PLATFORM_CPPFLAGS)

extensionsdir = $(libdir)/thunarx-1
extensions_LTLIBRARIES =						\
	thunar-svn-plugin.la

thunar_svn_plugin_la_SOURCES =						\
	tsp-svn-backend.c						\
	tso-svn-backend.h						\
	tsp-svn-action.c						\
	tso-svn-action.h						\
	tsp-provider.c							\
	tsp-provider.h							\
	thunar-svn-plugin.c

thunar_svn_plugin_la_CFLAGS =						\
	$(PLATFORM_CFLAGS)						\
	$(THUNARX_CFLAGS)						\
	$(THUNAR_VFS_CFLAGS)						\
	$(APR_CFLAGS)

thunar_svn_plugin_la_LDFLAGS =						\
	-avoid-version							\
	-export-dynamic							\
	-module								\
	$(PLATFORM_LDFLAGS)						\
	$(THUNARX_LDFLAGS)						\
	$(THUNAR_VFS_LDFLAGS)						\
	$(APR_LDFLAGS)

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
