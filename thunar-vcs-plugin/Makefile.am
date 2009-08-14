
INCLUDES =								\
	-I$(top_builddir)						\
	-I$(top_srcdir)							\
	-DG_LOG_DOMAIN=\"thunar-vcs-plugin\"				\
	-DEXO_API_SUBJECT_TO_CHANGE					\
	-DLIBEXECDIR=\"$(libexecdir)\"					\
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"				\
	$(PLATFORM_CPPFLAGS)

extensionsdir = $(libdir)/thunarx-2
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

thunar_vcs_plugin_la_CFLAGS =						\
	$(PLATFORM_CFLAGS)						\
	$(THUNARX_CFLAGS)						\
	$(EXO_CFLAGS)							\
	$(LIBXFCE4UTIL_CFLAGS)
if HAVE_SUBVERSION
thunar_vcs_plugin_la_CFLAGS +=						\
	$(APR_CFLAGS)							\
	-DTVP_SVN_HELPER=\"$(libexecdir)/tvp-svn-helper\"
endif
if HAVE_GIT
thunar_vcs_plugin_la_CFLAGS +=						\
	-DTVP_GIT_HELPER=\"$(libexecdir)/tvp-git-helper\"
endif

thunar_vcs_plugin_la_LDFLAGS =						\
	-avoid-version							\
	-export-dynamic							\
	-module								\
	$(PLATFORM_LDFLAGS)						\
	$(THUNARX_LDFLAGS)						\
	$(EXO_LDFLAGS)							\
	$(LIBXFCE4UTIL_LDFLAGS)
if HAVE_SUBVERSION
thunar_vcs_plugin_la_LDFLAGS +=						\
	$(APR_LDFLAGS)
endif

# vi:set ts=8 sw=8 noet ai nocindent:
