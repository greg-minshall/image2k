# $Id: fba8f98c312c5ae3a0a0bffd7bbedd95612a9b89 $

lib_LTLIBRARIES = libimage2k.la
libimage2k_la_SOURCES = image2k.c

bin_PROGRAMS = imagemean imagehist imagergb
imagemean_SOURCES = imagemean.c
imagehist_SOURCES = imagehist.c
imagergb_SOURCES = imagergb.c

AM_CPPFLAGS = ${IMLIB2_CFLAGS} ${MAGICKWAND_CFLAGS} ${X11_CFLAGS}
AM_LDFLAGS = ${IMLIB2_LIBS} ${MAGICKWAND_LIBS} ${X11_LIBS}

ACLOCAL_AMFLAGS = -I m4
