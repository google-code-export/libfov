EXTRA_DIST = Doxyfile Doxyheader.html Doxyfooter.html doxygen.css
CLEANFILES = *~

if HAVE_DOXYGEN
all: html
else
all:
endif

html: @top_srcdir@/fov/fov.h Doxyfile Doxyheader.html Doxyfooter.html doxygen.css
	(cat Doxyfile; echo "PROJECT_NUMBER=@LIBFOV_RELEASE@") | doxygen - > /dev/null

clean:
	$(RM) -r html *~
