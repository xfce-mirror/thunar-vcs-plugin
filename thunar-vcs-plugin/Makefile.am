
INCLUDES =								\
	-I$(top_builddir)						\
	-I$(top_srcdir)							\
	-DG_LOG_DOMAIN=\"thunar-vcs-plugin\"				\
	-DEXO_API_SUBJECT_TO_CHANGE					\
	-DLIBEXECDIR=\"$(libexecdir)\"					\
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"				\
	$(PLATFORM_CPPFLAGS)

extensionsdir = $(libdir)/thunarx-1
extensions_LTLIBRARIES =						\
	thunar-vcs-plugin.la

thunar_vcs_plugin_la_SOURCES =						\
	tvp-svn-backend.c						\
	tvp-svn-backend.h						\
	tvp-svn-action.c						\
	tvp-svn-action.h						\
	tvp-svn-property-page.c						\
	tvp-svn-property-page.h						\
	tvp-provider.c							\
	tvp-provider.h							\
	thunar-vcs-plugin.c

thunar_vcs_plugin_la_CFLAGS =						\
	$(PLATFORM_CFLAGS)						\
	$(THUNARX_CFLAGS)						\
	$(THUNAR_VFS_CFLAGS)						\
	$(APR_CFLAGS)							\
	-DTVP_SVN_HELPER=\"$(libexecdir)/tvp-svn-helper\"

thunar_vcs_plugin_la_LDFLAGS =						\
	-avoid-version							\
	-export-dynamic							\
	-module								\
	$(PLATFORM_LDFLAGS)						\
	$(THUNARX_LDFLAGS)						\
	$(THUNAR_VFS_LDFLAGS)						\
	$(APR_LDFLAGS)

# vi:set ts=8 sw=8 noet ai nocindent:
