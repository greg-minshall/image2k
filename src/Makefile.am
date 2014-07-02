# $Id: fba8f98c312c5ae3a0a0bffd7bbedd95612a9b89 $

lib_LTLIBRARIES = libimage2k.la
libimage2k_la_SOURCES = image2k.c rimage2k.c

EXTRA_PROGRAMS = imagemean imagehist imagergb
imagemean_SOURCES = imagemean.c image2k.c
imagehist_SOURCES = imagehist.c image2k.c
imagergb_SOURCES = imagergb.c image2k.c

AM_COPT = -g
AM_CFLAGS = -Wall

libimage2k_la_CFLAGS = ${R_CFLAGS} ${AM_CFLAGS}
libimage2k_la_CPPFLAGS = ${R_CPPFLAGS} ${AM_CPPFLAGS}
libimage2k_la_LDFLAGS = ${R_LDFLAGS} ${AM_LDFLAGS}
libimage2k_la_CC = ${R_CC}

# keep libtoolize happy...
# https://trac.transmissionbt.com/ticket/1335

ACLOCAL_AMFLAGS = -Ibuild-aux/m4

# from:
# http://stackoverflow.com/questions/13948916/how-to-build-so-module-in-automake-and-a-libtool-using-project
# (but, modified to 1: put links here, rather that top_builddir; 2:
# use all names from library_names (original only used ^dlname).)
.PHONY: convenience-link clean-convenience-link

convenience-link: $(lib_LTLIBRARIES)
	@for soname in `echo | $(EGREP) "^library_names=" $^ | $(SED) -e "s|^library_names=||" | $(SED) -e "s|'||g"`; do  \
		sonameminus=`echo $$soname | sed -e "s|^lib||"`; \
		echo "$$sonameminus: creating convenience link from $(abs_builddir)/.libs to $(builddir)"; \
		rm -f $(builddir)/$$sonameminus ; \
		test -e $(abs_builddir)/.libs/$$soname && \
		cd $(builddir) && \
		$(LN_S) $(abs_builddir)/.libs/$$soname $$sonameminus || true;\
	done 

clean-convenience-link:
	@for soname in `echo | $(EGREP) "^library_names=" $(lib_LTLIBRARIES) | $(SED) -e "s|^library_names=||" | $(SED) -e "s|'||g"`; do  \
		sonameminus=`echo $$soname | sed -e "s|^lib||"`; \
		echo "$$sonameminus: cleaning convenience links"; \
		test -L $(builddir)/$$sonameminus && rm -f $(builddir)/$$sonameminus || true; \
	done 

all-local:: convenience-link

clean-local:: clean-convenience-link
