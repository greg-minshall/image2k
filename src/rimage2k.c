/*
 * allow any image that imlib2 or magickwand can parse to be loaded
 * into an R pixmap.
 */


// XXX to create library: R CMD SHLIB -lImlib2 rimlib2.c
// XXX to see dependencies:  R CMD otool -L rimlib2.dylib
// XXX to re-do build stuff: autoreconf --install

#include <strings.h>

#include <R.h>
#include <Rinternals.h>
#include <R_ext/Rdynload.h>

#include "config.h"

#include "image2k.h"


/*
 * some helpful info on writing extensions to R:
 *
 * https://darrenjw.wordpress.com/2010/12/30/calling-c-code-from-r
 * http://www.r-project.org/doc/Rnews/Rnews_2001-3.pdf
 */



/* Rinlinedfuns.h:list5++ */
static SEXP
list6(SEXP s, SEXP t, SEXP u, SEXP v, SEXP w, SEXP x)
{
    PROTECT(s);
    s = CONS(s, list5(t, u, v, w, x));
    UNPROTECT(1);
    return s;
}

/*
 * given the name of an image file, return some of its metadata and
 * its R, G, and B channels.
 */
static SEXP
rimageall(SEXP args) {
    const char *file;
    SEXP xfile, sr, sg, sb, rval, names;
    Imlib_Image x;              /* imlib2 context */
    Imlib_Load_Error error_return;
    DATA32 *data;
    double www, hhh, maxval, *r, *g, *b;
    int type, bytes, i, val;
    char *imformat;

    xfile= PROTECT(coerceVector(CADR(args), STRSXP)); /* 1 */
    file = CHAR(STRING_ELT(xfile, 0));

    x = imlib_load_image_with_error_return(file, &error_return);
    if (x == NULL) {
        Imlib_Load_Error error_return;
        error("unable to open \"%s\": %s\n",
              file, image_decode_load_error(error_return));
        /*NOTREACHED*/
    }
    imlib_context_set_image(x);
    imformat = imlib_image_format();
    printf("imformat=\"%s\"\n", imformat); /* XXX what is this information? */

    www = imlib_image_get_width();
    hhh = imlib_image_get_height();
    maxval = IMLIB_MAX_PIXEL;

    bytes = www*hhh;

    sr = PROTECT(allocVector(REALSXP, bytes)); /* 2 */
    sg = PROTECT(allocVector(REALSXP, bytes)); /* 3 */
    sb = PROTECT(allocVector(REALSXP, bytes)); /* 4 */

    r = REAL(sr);
    g = REAL(sg);
    b = REAL(sb);

    // now, map data, copy it to the R structure, and free it
    data = imlib_image_get_data_for_reading_only();
    for (i = 0; i < bytes; i++) {
        val = data[i];
        r[i] = GetR(val);
        g[i] = GetG(val);
        b[i] = GetB(val);
    }
    imlib_free_image_and_decache();

    // now, put together the return: height, width, r, g, b
    rval = PROTECT(list6(PROTECT(ScalarReal(hhh)), /* 5 */
                         PROTECT(ScalarReal(www)), /* 6 */
                         PROTECT(ScalarReal(maxval)), /* 7 */
                         sr,
                         sg,
                         sb)); /* 8 */
    names = PROTECT(list6(PROTECT(mkString("nr")), /* 9,10 */
                          PROTECT(mkString("nc")), /* 11 */
                          PROTECT(mkString("maxval")), /* 12 */
                          PROTECT(mkString("red")),   /* 13 */
                          PROTECT(mkString("green")),   /* 14 */
                          PROTECT(mkString("blue")))); /* 15 */
    namesgets(rval, names);

    UNPROTECT(15);
    return rval;
}

/* R glue */

static const
R_ExternalMethodDef externalMethods[] = {
   {"rimageread",  (DL_FUNC) &rimageread, 1},
   {"rimageread",  (DL_FUNC) &rimagewrite, 1},
   NULL
};

void R_init_myRoutines(DllInfo *info)
{
 /* Register the .External routine.
  * No .C, .Call, or Fortran routines,
  * so pass those arrays as NULL.
  */
 R_registerRoutines(info,
                    NULL, NULL,
                    NULL, externalMethods);
}
