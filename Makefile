# $Id$

COPT = -O2
CINCLUDES = -I/sw/include -I/usr/X11R6/include
CFLAGS = ${COPT} ${CINCLUDES}

LDFLAGS = -L/sw/lib -limlib2 ${CFLAGS}

images: imagemean imagehist imagergb

imagemean: imagemean.o

imagemean.o: imagemean.c image.h

imagehist: imagehist.o image.h

imagehist.o: imagehist.c image.h

clean:
	rm -f imagemean imagemean.o imagehist imagehist.o imagergb imagergb.o
