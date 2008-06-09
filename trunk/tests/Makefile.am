CLEANFILES = *~

INCLUDES = -I@top_srcdir@
AM_CFLAGS = -g -O2 -ansi -pedantic -pedantic-errors -Wfloat-equal
AM_CXXFLAGS = $(AM_CFLAGS)

noinst_PROGRAMS = fovtest
fovtest_SOURCES = fovtest.cc
fovtest_LDADD = @top_srcdir@/fov/libfov.la
