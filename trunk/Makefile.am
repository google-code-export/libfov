
SUBDIRS = fov tests examples docs
CLEANFILES = *~
EXTRA_DIST=autogen.sh
ACLOCAL_AMFLAGS=-I m4

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libfov.pc

release: dist
	lftp -e 'put libfov-$(LIBFOV_RELEASE).tar.gz' -u anonymous ftp://upload.sourceforge.net/incoming

upload_docs:
	$(MAKE) -C docs
	if [ -f docs/html/index.html ]; then \
		rsync --delete -rve ssh docs/html/ $(USER)@shell.sourceforge.net:/home/groups/l/li/libfov/htdocs/api; \
	fi
