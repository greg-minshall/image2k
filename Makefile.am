SUBDIRS = src
TMP = __TMP__

# we md5 ${RGOLDIN} into ${RGOLDOUT}.  this should only be run if
# something massive changes.  (in fact, ${RGOLDOUT} is under source
# control.)

RGOLDIN = tests/c-L1001745.png
RGOLDOUT = tests/c-L1001745-png.digest

RMD5IMAGE = tests/rmd5image

# keep libtoolize happy...
# https://trac.transmissionbt.com/ticket/1335

ACLOCAL_AMFLAGS = -Ibuild-aux/m4

 # XXX man/image2k-package.Rd (produced from %.in by ./configure) is
 # needed to keep R CMD check from having problems (while we have to
 # use the hack in rbuild below)
EXTRA_DIST = DESCRIPTION LICENSE NAMESPACE \
		.Rbuildignore \
		R/image2k.R \
		inst/CITATION \
		man/image2k-package.Rd \
		tests/

${RGOLDOUT}: ${RGOLDIN}
	${RMD5IMAGE} -f $^ 2> /dev/null > $@

RPACKAGE = ${PACKAGE}_${VERSION}
RTARFILE = ${RPACKAGE}.tar
RTARFILEGZ = ${RTARFILE}.gz

# in case someone wants to mess with any of these
R = R
TAR = tar


# i'd like to do testing with some sort of test harness (such as
# https://github.com/sstephenson/bats), but obviously it's not going
# to exist on all platforms...)  otoh, users don't need this
# functionality (maybe).  also, maybe autotools will propagate it for
# me?

${RTARFILEGZ}: ${PACKAGE}-${VERSION}.tar.gz
	echo ${PACKAGE}
	from=`pwd` && \
	rm -rf ${TMP} && \
	${MKDIR_P} ${TMP} && \
	cd ${TMP} && \
	${TAR} xzf $${from}/$^ && \
	mv ${PACKAGE}-${VERSION} ${PACKAGE} && \
	${R} CMD build ${PACKAGE} && mv $@ $${from}; \
	cd $${from};		# we're here, but for clarity...
	rm -rf ${TMP}
	echo XXX hack to get around man/image2k-package.Rd.in not being included && \
	from=`pwd` && \
	rm -rf ${TMP} && \
	${MKDIR_P} ${TMP} && \
	cd ${TMP} && \
	${TAR} xzf $${from}/$@ && \
	cp -p $${from}/man/image2k-package.Rd.in ${PACKAGE}/man/ && \
	${TAR} czf $${from}/$@ ${PACKAGE}
	echo XXX hack done && \
	cd $${bname};		# we're here, but for clarity...

rbuild: ${PACKAGE}_${VERSION}.tar.gz

rcheck: rbuild
	from=`pwd` && \
	rm -rf ${TMP} && \
	${MKDIR_P} ${TMP} && \
	cd ${TMP} && \
	${R} CMD check $${from}/${RTARFILEGZ}
	cd $${from};		# we're here, but for clarity...
	rm -rf ${TMP}

rtests: rbuild
	from=`pwd` && \
	rm -rf ${TMP} && \
	${MKDIR_P} ${TMP} && \
	cd ${TMP} && \
	${MKDIR_P} testlib && \
	echo "checking install" && \
	${R} CMD install --library=testlib $${from}/${RTARFILEGZ} && \
	echo "checking functionality" && \
	$${from}/${RMD5IMAGE} --image2klib testlib \
			-f $${from}/tests/c-L1001745.png 2>&1 | \
		${GREP} -v "^libpng warning" | \
		diff $${from}/${RGOLDOUT} - && \
	rm -rf ${TMP}

rinstall: rbuild
	${R} CMD install ${RTARFILEGZ}
