# $Id$

COPT = -O2
CINCLUDES = -I/sw/include -I/usr/X11R6/include
CFLAGS = ${COPT} ${CINCLUDES}

LDFLAGS = -L/sw/lib -limlib2 ${CFLAGS}

images: imagemean imagehist imagergb

imagemean: imagemean.o imagesubr.o

imagemean.o: imagemean.c imageutils.h

imagehist: imagehist.o imagesubr.o

imagehist.o: imagehist.c imageutils.h

imagergb: imagergb.o imagesubr.o

imagergb.o: imagergb.c imageutils.h

clean:
	rm -f imagemean imagemean.o imagehist imagehist.o imagergb imagergb.o
