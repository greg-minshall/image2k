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

# i'd like to do testing with some sort of test harness (such as
# https://github.com/sstephenson/bats), but obviously it's not going
# to exist on all platforms...)  otoh, users don't need this
# functionality (maybe).  also, maybe autotools will propagate it for
# me?
distcheck-hook:
	pwd
	echo distdir=${distdir}

${PACKAGE}_${VERSION}.tar.gz: ${PACKAGE}-${VERSION}.tar.gz
	cur=`pwd` && \
	rm -rf ${TMP} && \
	mkdir ${TMP} && \
	cd ${TMP} && \
	tar xzf $${cur}/$^ && \
	mv ${PACKAGE}-${VERSION} ${PACKAGE} && \
	R CMD build ${PACKAGE} && mv $@ $${cur};
	cd $${cur};		# we're here, but for documentation...
	rm -rf ${TMP}

rbuild: ${PACKAGE}_${VERSION}.tar.gz

rcheck: rbuild
	cd ${TMP} && \
	R CMD check ${PACKAGE}
