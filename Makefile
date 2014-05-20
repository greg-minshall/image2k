# $Id$

COPT = -O2
CINCLUDES = -I/sw/include -I/usr/X11R6/include
CFLAGS = ${COPT} ${CINCLUDES}

LDFLAGS = -L/sw/lib -limlib2 ${CFLAGS}

imagemean: imagemean.o

imagemean.o: imagemean.c

clean:
	rm imagemean.o
