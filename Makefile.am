SUBDIRS = src
TMP = __TMP__

# keep libtoolize happy...
# https://trac.transmissionbt.com/ticket/1335

ACLOCAL_AMFLAGS = -Ibuild-aux/m4

EXTRA_DIST = DESCRIPTION LICENSE NAMESPACE \
		.Rbuildignore \
		R/image2k.R \
		inst/CITATION \
		man/image2k-package.Rd \
		tests/

distcheck-hook:
	pwd
	echo distdir=${distdir}

${PACKAGE}_${VERSION}.tar.gz: ${PACKAGE}-${VERSION}.tar.gz
	cur=`pwd`; \
	rm -rf ${TMP}; \
	mkdir ${TMP}; \
	cd ${TMP}; \
	tar xzf $${cur}/$^; \
	mv ${PACKAGE}-${VERSION} ${PACKAGE}; \
	R CMD build ${PACKAGE} && mv $@ $${cur}; \
	cd $${cur};
	echo rm -rf ${TMP}

rcheck: ${PACKAGE}_${VERSION}.tar.gz
	cd ${TMP} ; \
	R CMD check ${PACKAGE}
