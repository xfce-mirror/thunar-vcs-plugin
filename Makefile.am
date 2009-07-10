
SUBDIRS =								\
	icons								\
	po								\
	thunar-vcs-plugin
if HAVE_SUBVERSION
SUBDIRS += tvp-svn-helper
endif
if HAVE_GIT
SUBDIRS += tvp-git-helper
endif

distclean-local:
	rm -rf *.spec *.cache *~

rpm: dist
	rpmbuild -ta $(PACKAGE)-$(VERSION).tar.gz
	@rm -f $(PACKAGE)-$(VERSION).tar.gz

# vi:set ts=8 sw=8 noet ai nocindent:
