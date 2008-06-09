CLEANFILES = *~
AM_CFLAGS = -Wall -O2 -ansi -pedantic -pedantic-errors -Wfloat-equal -Werror

library_includedir=$(includedir)/$(LIBFOV_LIBRARY_NAME)
library_include_HEADERS = fov.h

libfov_configdir = $(libdir)/$(LIBFOV_LIBRARY_NAME)/include
libfov_config_DATA = config.h

lib_LTLIBRARIES = libfov.la
libfov_la_SOURCES = fov.c
libfov_la_LIBS = $(LIBM)
libfov_la_LDFLAGS= -version-info $(LIBFOV_LTVERSION)

splint:
	splint fov.c
