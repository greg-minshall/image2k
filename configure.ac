AC_INIT([imageutils],[0.1])

AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_CONFIG_MACRO_DIR([m4])
AM_PROG_AR
LT_INIT

AC_CONFIG_SRCDIR([src])

# from Writing R Extensions: get the right compiler and flags
: ${R_HOME=`R RHOME`}
if test -z "${R_HOME}"; then
  echo "could not determine R_HOME"
  exit 1
fi
CC=`"${R_HOME}/bin/R" CMD config CC`
CFLAGS=`"${R_HOME}/bin/R" CMD config CFLAGS`
CPPFLAGS=`"${R_HOME}/bin/R" CMD config CPPFLAGS`
LDFLAGS=`"${R_HOME}/bin/R" CMD config LDFLAGS`


AC_PROG_CC
AC_PROG_CPP

# a good reference to PKG_CHECK_MODULES:
# https://www.flameeyes.eu/autotools-mythbuster/pkgconfig/pkg_check_modules.html

have_imlib2=yes
PKG_CHECK_MODULES([IMLIB2], [imlib2], [AC_DEFINE([HAVE_IMLIB2], [1], ["Have Imlib2"])], [have_imlib2=no])
AC_SUBST(IMLIB2_CFLAGS)
AC_SUBST(IMLIB2_LIBS)

# this ImageMagick/MagickWand stuff is for some future, mythical time
# when i will modify the programs to use both libraries.

have_magickwand=yes
PKG_CHECK_MODULES([MAGICKWAND], [MagickWand], [AC_DEFINE([HAVE_MAGICKWAND], [1], ["Have MagickWand"])], [have_magickwand=no])
AC_SUBST(MAGICKWAND_CFLAGS)
AC_SUBST(MAGICKWAND_LIBS)

# error if we can find neither Imlib2 nor MagickWand...
if test x${have_imlib2} == xno && test x${have_magickwand} == xno; then
   AC_MSG_ERROR([
   ---------------------------------------------------
   Neither Imlib2 nor MagickWand (an optional part of
   ImageMagick) are available on this system (as
   configured).  Unable to build this package.
   ---------------------------------------------------])
fi


PKG_CHECK_MODULES([X11], [x11])
AC_SUBST(X11_CFLAGS)
AC_SUBST(X11_LIBS)

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
        Makefile
        src/Makefile
])

AC_OUTPUT
