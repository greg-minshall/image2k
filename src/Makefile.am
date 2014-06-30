# $Id: fba8f98c312c5ae3a0a0bffd7bbedd95612a9b89 $

lib_LTLIBRARIES = librimage2k.la
librimage2k_la_SOURCES = image2k.c rimage2k.c

bin_PROGRAMS = imagemean imagehist imagergb
imagemean_SOURCES = imagemean.c image2k.c
imagehist_SOURCES = imagehist.c image2k.c
imagergb_SOURCES = imagergb.c image2k.c

AM_COPT = -g
AM_CFLAGS = -Wall

librimage2k_la_CFLAGS = ${R_CFLAGS} ${AM_CFLAGS}
librimage2k_la_CPPFLAGS = ${R_CPPFLAGS} ${AM_CPPFLAGS}
librimage2k_la_LDFLAGS = ${R_LDFLAGS} ${AM_LDFLAGS}
librimage2k_la_CC = ${R_CC}

