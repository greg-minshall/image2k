# initialize things
AC_INIT([image2k],[0.1])
AC_CONFIG_AUX_DIR(build-aux)
AC_CONFIG_MACRO_DIR([build-aux/m4])
AC_CONFIG_SRCDIR([src/image2k.c])

AM_INIT_AUTOMAKE([-Wall -Werror foreign])

# declare some tools
AC_PROG_CC
AC_PROG_CPP
AM_PROG_AR

AC_PROG_GREP
AC_PROG_MKDIR_P

# initialize libtool (must be done *after* AM_PROG_AR)
LT_INIT


# from Writing R Extensions: get the right compiler and flags for R

: ${R_HOME=`R RHOME`}
if test -z "${R_HOME}"; then
  echo "could not determine R_HOME"
  exit 1
fi
R_CC=`"${R_HOME}/bin/R" CMD config CC`
R_CFLAGS=`"${R_HOME}/bin/R" CMD config CFLAGS`
R_CPPFLAGS=`"${R_HOME}/bin/R" CMD config --cppflags`
R_LDFLAGS=`"${R_HOME}/bin/R" CMD config --ldflags`
AC_SUBST(R_CC)
AC_SUBST(R_CFLAGS)
AC_SUBST(R_CPPFLAGS)
AC_SUBST(R_LDFLAGS)


# now, check for Imlib2 and/or MagickWand.

# a good reference to PKG_CHECK_MODULES:
# https://www.flameeyes.eu/autotools-mythbuster/pkgconfig/pkg_check_modules.html

have_imlib2=1
PKG_CHECK_MODULES([IMLIB2],
                  [imlib2],
                  [AC_DEFINE([HAVE_IMLIB2], [1], [Have Imlib2])],
                  [have_imlib2=0])
AC_SUBST(IMLIB2_CFLAGS)
AC_SUBST(IMLIB2_LIBS)
CPPFLAGS="${CPPFLAGS} ${IMLIB2_CFLAGS}"
LIBS="${LIBS} ${IMLIB2_LIBS}"
AC_SUBST(have_imlib2)

have_magickwand=1
PKG_CHECK_MODULES([MAGICKWAND],
                  [MagickWand],
                  [AC_DEFINE([HAVE_MAGICKWAND], [1], [Have MagickWand])],
                  [have_magickwand=0])
AC_SUBST(MAGICKWAND_CFLAGS)
AC_SUBST(MAGICKWAND_LIBS)
CPPFLAGS="${CPPFLAGS} ${MAGICKWAND_CFLAGS}"
LIBS="${LIBS} ${MAGICKWAND_LIBS}"
AC_SUBST(have_magickwand)

# error if we can find neither Imlib2 nor MagickWand...
if test x${have_imlib2} == xno && test x${have_magickwand} == xno; then
   AC_MSG_ERROR([
   ---------------------------------------------------
   Neither Imlib2 nor MagickWand (an optional part of
   ImageMagick) are available on this system (as
   configured).  Unable to build this package.
   ---------------------------------------------------])
fi


# need X11, as some of the include files (in MagickWand, i think) try
# to include from there.

AC_PATH_X
if test -n "${x_includes}"; then
  X_CFLAGS="-I${x_includes}"
  AC_SUBST(X_CFLAGS)
  CPPFLAGS="${CPPFLAGS} -I${x_includes}"
fi
if test -n "${x_libraries}"; then
  X_LIBS="-L${x_libraries} -lX11"
  AC_SUBST(X_LIBS)
  LIBS="${LIBS} -L${x_libraries} -lX11"
else
  LIBS="${LIBS} -lX11"
fi


# declare output files

AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_FILES([
        DESCRIPTION
        Makefile
        man/image2k-package.Rd
        R/image2k.R
        src/Makefile
        tests/rmd5image.R
])


# create the output
AC_OUTPUT
