/*
 * allow any image that imlib2 can parse to be loaded into an R pixmap.
 */


// XXX to create library: R CMD SHLIB -lImlib2 rimlib2.c
// XXX to see dependencies:  R CMD otool -L rimlib2.dylib
// XXX to re-do build stuff: autoreconf --install

#include <strings.h>

#include <Imlib2.h>

#include <R.h>
#include <Rinternals.h>
#include <R_ext/Rdynload.h>

#define IMLIB_MAX_PIXEL 255

/*
 * some helpful info on writing extensions to R:
 *
 * https://darrenjw.wordpress.com/2010/12/30/calling-c-code-from-r
 * http://www.r-project.org/doc/Rnews/Rnews_2001-3.pdf
 */


/*
 * These are for use with Imlib2.h, which keeps its data (as returned
 * from imlib_image_get_data() and friends) in this format:

"The image data is returned in the format of a DATA32 (32 bits) per
pixel in a linear array ordered from the top left of the image to the
bottom right going from left to right each line. Each pixel has the
upper 8 bits as the alpha channel and the lower 8 bits are the blue
channel - so a pixel's bits are ARGB (from most to least significant,
8 bits per channel)."

 * (from the documentation for imlib_image_get_data())
 */

#define GetA(argb) (((argb)>>24)&0xff)
#define GetR(argb) (((argb)>>16)&0xff)
#define GetG(argb) (((argb)>> 8)&0xff)
#define GetB(argb) (((argb)    )&0xff)

#define PutA(argb, a) (((argb)&0x00ffffff) | (((a)<<24)&0xff000000))
#define PutR(argb, b) (((argb)&0xff00ffff) | (((b)<<16)&0x00ff0000))
#define PutG(argb, g) (((argb)&0xffff00ff) | (((g)<< 8)&0x0000ff00))
#define PutB(argb, r) (((argb)&0xffffff00) | (((r)    )&0x000000ff))

// return a string "explainig" the reason a load_image (or save_image)
// failed.
static char *
image_decode_load_error(Imlib_Load_Error error) {
    switch (error) {
    case IMLIB_LOAD_ERROR_NONE:
        return "imlib_load_error_none";
    case IMLIB_LOAD_ERROR_FILE_DOES_NOT_EXIST:
        return "imlib_load_error_file_does_not_exist";
    case IMLIB_LOAD_ERROR_FILE_IS_DIRECTORY:
        return "imlib_load_error_file_is_directory";
    case IMLIB_LOAD_ERROR_PERMISSION_DENIED_TO_READ:
        return "imlib_load_error_permission_denied_to_read";
    case IMLIB_LOAD_ERROR_NO_LOADER_FOR_FILE_FORMAT:
        return "imlib_load_error_no_loader_for_file_format";
    case IMLIB_LOAD_ERROR_PATH_TOO_LONG:
        return "imlib_load_error_path_too_long";
    case IMLIB_LOAD_ERROR_PATH_COMPONENT_NON_EXISTANT:
        return "imlib_load_error_path_component_non_existant";
    case IMLIB_LOAD_ERROR_PATH_COMPONENT_NOT_DIRECTORY:
        return "imlib_load_error_path_component_not_directory";
    case IMLIB_LOAD_ERROR_PATH_POINTS_OUTSIDE_ADDRESS_SPACE:
        return "imlib_load_error_path_points_outside_address_space";
    case IMLIB_LOAD_ERROR_TOO_MANY_SYMBOLIC_LINKS:
        return "imlib_load_error_too_many_symbolic_linksn";
    case IMLIB_LOAD_ERROR_OUT_OF_MEMORY:
        return "imlib_load_error_out_of_memory";
    case IMLIB_LOAD_ERROR_OUT_OF_FILE_DESCRIPTORS:
        return "imlib_load_error_out_of_file_descriptors";
    case IMLIB_LOAD_ERROR_PERMISSION_DENIED_TO_WRITE:
        return "imlib_load_error_permission_denied_to_write";
    case IMLIB_LOAD_ERROR_OUT_OF_DISK_SPACE:
        return "imlib_load_error_out_of_disk_space";
    case IMLIB_LOAD_ERROR_UNKNOWN:
        return "imlib_load_error_unknown";
    default:
        return "unknown/invalid error";
    }
}

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
   {"rimageall",  (DL_FUNC) &rimageall, 1},
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
