AC_INIT([imageutils],[0.1])

AM_INIT_AUTOMAKE([-Wall -Werror foreign])

AC_LANG([C])
AC_PROG_CC

# a good reference to PKG_CHECK_MODULES:
# https://www.flameeyes.eu/autotools-mythbuster/pkgconfig/pkg_check_modules.html

PKG_CHECK_MODULES([IMLIB2], [imlib2], [AC_DEFINE([HAVE_IMLIB2], [1], ["Have Imlib2"])])
AC_SUBST(IMLIB2_CFLAGS)
AC_SUBST(IMLIB2_LIBS)

# this ImageMagick/MagickWand stuff is for some future, mythical time
# when i will modify the programs to use both libraries.
PKG_CHECK_MODULES([MAGICKWAND], [MagickWand], [AC_DEFINE([HAVE_MAGICKWAND], [1], ["Have MagickWand"])])
AC_SUBST(MAGICKWAND_CFLAGS)
AC_SUBST(MAGICKWAND_LIBS)

PKG_CHECK_MODULES([X11], [x11])
AC_SUBST(X11_CFLAGS)
AC_SUBST(X11_LIBS)

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
        Makefile
])

AC_OUTPUT
