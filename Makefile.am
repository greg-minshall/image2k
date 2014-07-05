SUBDIRS = src
TMP = __TMP__

# we md5 ${RGOLDIN} into ${RGOLDOUT}.  this should only be run if
# something massive changes.  (in fact, ${RGOLDOUT} is under source
# control.)

RGOLDIN = tests/c-L1001745.png
RGOLDOUT = tests/c-L1001745.digest

# keep libtoolize happy...
# https://trac.transmissionbt.com/ticket/1335

ACLOCAL_AMFLAGS = -Ibuild-aux/m4

EXTRA_DIST = DESCRIPTION LICENSE NAMESPACE \
		.Rbuildignore \
		R/image2k.R \
		inst/CITATION \
		man/image2k-package.Rd \
		tests/

${RGOLDOUT}: ${RGOLDIN}
	./rtests.R $^ 2> /dev/null > $@

# i'd like to do testing with some sort of test harness (such as
# https://github.com/sstephenson/bats), but obviously it's not going
# to exist on all platforms...)  otoh, users don't need this
# functionality (maybe).  also, maybe autotools will propagate it for
# me?
distcheck-hook:
	pwd
	echo distdir=${distdir}

${PACKAGE}_${VERSION}.tar.gz: ${PACKAGE}-${VERSION}.tar.gz
	from=`pwd` && \
	rm -rf ${TMP} && \
	mkdir ${TMP} && \
	cd ${TMP} && \
	tar xzf $${from}/$^ && \
	mv ${PACKAGE}-${VERSION} ${PACKAGE} && \
	R CMD build ${PACKAGE} && mv $@ $${from};
	cd $${from};		# we're here, but for clarity...
	rm -rf ${TMP}

rbuild: ${PACKAGE}_${VERSION}.tar.gz

rcheck: rbuild
	from=`pwd` && \
	rm -rf ${TMP} && \
	mkdir ${TMP} && \
	cd ${TMP} && \
	R CMD check $${from}/${PACKAGE}_${VERSION}.tar.gz
	cd $${from};		# we're here, but for clarity...
	rm -rf ${TMP}

rtestinstall: rbuild
	from=`pwd` && \
	rm -rf ${TMP} && \
	mkdir ${TMP} && \
	cd ${TMP} && \
	mkdir testlib && \
	R CMD install --library=testlib $${from}/${PACKAGE}_${VERSION}.tar.gz && \
	cd $${from};		# we're here, but for clarity...
	echo rm -rf ${TMP}


rtests: rbuild
	from=`pwd` && \
	cd ${TMP} && \
	$${from}/rtests.R --image2klib testlib $${from}/tests/c-L1001745.png \
						2> /dev/null | \
	(cd $${from}; diff ${RGOLDOUT} -);
