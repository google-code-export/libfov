CLEANFILES = *~

if HAVE_SDL
SDL_PROGS = simple
else
SDL_PROGS = 
endif

if HAVE_CURSES
CURSES_PROGS = 
CURSES_CFLAGS = -I/usr/include
CURSES_LIBS = -lcurses
else
CURSES_PROGS = 
endif

INCLUDES = -I@top_srcdir@

noinst_PROGRAMS = $(SDL_PROGS) $(CURSES_PROGS)

AM_CFLAGS = @SDL_CFLAGS@ -O2 -ansi -pedantic -Wall -pedantic-errors -Wfloat-equal -Werror -Wno-unused
AM_CXXFLAGS = $(AM_CFLAGS)

simple_LDADD = @top_srcdir@/fov/libfov.la @SDL_LIBS@
simple_SOURCES = \
simple.cc \
display.h \
display_sdl.c \
display_sdl.h \
map.cc \
map.h

#multi_LDADD = @top_srcdir@/fov/libfov.la
#multi_SOURCES = \
#multi.c \
#display_sdl.c \
#display_sdl.h
