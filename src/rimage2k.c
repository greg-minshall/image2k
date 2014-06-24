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

// #include "config.h"

#include "image2k.h"


/*
 * some helpful info on writing extensions to R:
 *
 * https://darrenjw.wordpress.com/2010/12/30/calling-c-code-from-r
 * http://www.r-project.org/doc/Rnews/Rnews_2001-3.pdf
 */

/*
 * this is what we use as a "cookie" for calls to/from image2k
 */

typedef struct {
    char id[16];                /* should be "image2k" */
    unsigned int www,
        hhh,
        len,
        depth;
    int protected;              /* how many R variables are PROTECTED
                                 * here */
    SEXP sr, sg, sb, sa;        /* arrays of red, green, blue, and alpha */
    int inited;
} mytype_t, *mytype_p;
#define MYTYPE_ID "image2k"

/*
 * cookie management
 */

static mytype_p
mytypecreate() {
    mytype_p mp;

    mp = (mytype_p)malloc(sizeof *mp);
    if (mp == 0) {
        error("%s:%d: unable to allocate %d bytes of memory",
              __FILE__, __LINE__, sizeof *mp);
        /*NOTREACHED*/
    }
    bzero(mp, sizeof *mp);

    strcat(mp->id, MYTYPE_ID);
    return mp;
}

static void
mytypedestroy(mytype_p mp) {
    free(mp);
}
        

static mytype_p
void2mytype(void *cookie) {
    mytype_p mp = (mytype_p) cookie;

    if ((mp == NULL) || strcmp(mp->id, MYTYPE_ID)) {
        error("%s:%d: problems with cookies", __FILE__, __LINE__);
        /*NOTREACHED*/
    }
    return mp;
}

static void *
mytype2void(mytype_p mytype) {
    return (void *) mytype;
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

static SEXP
list7(SEXP s, SEXP t, SEXP u, SEXP v, SEXP w, SEXP x, SEXP y)
{
    PROTECT(s);
    s = CONS(s, list6(t, u, v, w, x, y));
    UNPROTECT(1);
    return s;
}

/*
 * callouts from image2k
 */

/*
 * initialize data structures
 */

static void
init(mytype_p mp,
     unsigned int height, unsigned int width, unsigned int passed_depth) {
    int bytes;

    mp->hhh = height;
    mp->www = width;
    mp->len = mp->www*mp->hhh;
    mp->depth = passed_depth;

    bytes = mp->www*mp->hhh;

    mp->sr = PROTECT(allocVector(REALSXP, bytes)); /* 1 */
    mp->sg = PROTECT(allocVector(REALSXP, bytes)); /* 2 */
    mp->sb = PROTECT(allocVector(REALSXP, bytes)); /* 3 */
    mp->sa = PROTECT(allocVector(REALSXP, bytes)); /* 4 */

    mp->protected += 4;

    mp->inited = 1;
}

static void
fhw(void *cookie, const char *file,
    unsigned int height, unsigned int width, unsigned int depth) {
    mytype_p mp = void2mytype(cookie);

    if (!mp->inited) {
        init(mp, height, width, depth);
    }
}


static void
addpixel(void *cookie, int i, float red, float green, float blue, float alpha) {
    mytype_p mp = void2mytype(cookie);
    double *r, *g, *b, *a;

    r = REAL(mp->sr);
    g = REAL(mp->sg);
    b = REAL(mp->sb);
    a = REAL(mp->sa);

#if 0
    fprintf(stderr, "%d %f %f %f %f\n", i, red, green, blue, alpha);
#endif /* 0 */
    r[i] = red;
    g[i] = green;
    b[i] = blue;
    a[i] = alpha;
}


#if 0
static void
getpixels(void *cookie, int i,
          float *pred, float *pgreen, float *pblue, float *palpha) {
    mytype_p mp = void2mytype(cookie);

    // now, map data, copy it to the R structure, and free it
    data = imlib_image_get_data_for_reading_only();
    for (i = 0; i < bytes; i++) {
        val = data[i];
        r[i] = GetR(val);
        g[i] = GetG(val);
        b[i] = GetB(val);
    }
    imlib_free_image_and_decache();

    *pred = mp->rpixels[i];
    *pgreen = mp->gpixels[i];
    *pblue = mp->bpixels[i];
    *palpha = mp->apixels[i];
}
#endif

/*
 * given the name of an image file, return some of its metadata and
 * its R, G, and B channels.
 */

static SEXP
rimageread(SEXP args) {
    const char *file;
    SEXP xfile, rval, names;
    mytype_p mp;
    readfile_t readfile;
    int bytes, protected = 0;

    /* which should we use, Imlib2 or MagicWand? */
#if defined(HAVE_IMLIB2) && defined(HAVE_MAGICKWAND)
    if (flagk) {
        readfile = readfilek;
    } else {
        readfile = readfile2;
    }
#elif defined(HAVE_IMLIB2)
    readfile = readfile2;
#elif defined(HAVE_MAGICKWAND)
    readfile = readfilek;
#else
// this should not occur!
#error Need Imlib2 or ImageMagick -- neither defined at compilation time
#endif /* defined(HAVE_IMLIB2) && defined(HAVE_MAGICKWAND) */

    xfile = PROTECT(coerceVector(CADR(args), STRSXP)); /* 1 */
    protected++;
    file = CHAR(STRING_ELT(xfile, 0));

    mp = mytypecreate();

    /* read in the image, filling in *mp as a side effect */
    (readfile)(mp, file, fhw, addpixel);

    // now, put together the return: height, width, r, g, b
    rval = PROTECT(list7(PROTECT(ScalarReal(mp->hhh)),    /* 1,2 */
                         PROTECT(ScalarReal(mp->www)),    /* 3 */
                         PROTECT(ScalarReal(exp2(mp->depth))), /* 4 */
                         mp->sr,
                         mp->sg,
                         mp->sb,
                         mp->sa));
    protected += 4;
    names = PROTECT(list7(PROTECT(mkString("nr")),     /* 1,2 */
                          PROTECT(mkString("nc")),     /* 3 */
                          PROTECT(mkString("maxval")), /* 4 */
                          PROTECT(mkString("red")),    /* 5 */
                          PROTECT(mkString("green")),  /* 6 */
                          PROTECT(mkString("blue")),   /* 7 */
                          PROTECT(mkString("alpha")))); /* 8 */
    protected += 8;
    namesgets(rval, names);

    protected += mp->protected;
    mytypedestroy(mp);

    UNPROTECT(protected);

    return rval;
}

static SEXP
rimagewrite(SEXP args) {
    const char *file;
    SEXP xfile, rval, names;
    mytype_p mp;
    writefile_t writefile;
    int bytes, protected = 0;
}

/* R glue */

static const
R_ExternalMethodDef externalMethods[] = {
   {"rimageread",  (DL_FUNC) &rimageread, 1},
   {"rimagewrite",  (DL_FUNC) &rimagewrite, 1},
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
