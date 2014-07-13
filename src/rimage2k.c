/*
 * allow any image that imlib2 or magickwand can parse to be loaded
 * into an R pixmap.
 */

/* XXX should i start at 1?  or, at 0? */


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
    double *rr, *rg, *rb, *ra;  /* pointers into sr, sg, sb, sa */
    int inited;
} mytype_t, *mytype_p;
#define MYTYPE_ID "image2k"

/*
 * cookie management
 */

static mytype_p
mytypecreate() {
    mytype_p mp;

    mp = (mytype_p)R_alloc(1, sizeof *mp);
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
    /*
     * we're using R_alloc, so R will the memory when we
     * exit back to R
     */
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


static int
transposei(mytype_p mp, int i) {
    int ir, ic, j;
    static int count = 0;

    ir = i/mp->www;
    ic = i-(ir*mp->www);
    j = (ic*mp->hhh)+ir;
#if 0
    if (count%1011 == 1) {      /* i always get this computation wrong... */
        fprintf(stderr, "www %d, hhh %d, i %d, ir %d, ic %d, j %d\n",
                mp->www, mp->hhh, i, ir, ic, j);
    }
#endif /* 0 */
    count++;
    
    return j;
}


static int
myerrprint(const char *restrict format, ...) {
    va_list ap;

    va_start(ap, format);
    REvprintf(format, ap);
    va_end(ap);
    return(0);                  /* we can't [easily] match the spec... */
}

static void
myexit(int status) {
    error("exiting (%d)\n", status);
    /*NOTREACHED*/
}

static void *
mymalloc(size_t size) {
    return(R_alloc(size, 1));
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

    mp->hhh = height;
    mp->www = width;
    mp->len = mp->www*mp->hhh;
    mp->depth = passed_depth;

    mp->sr = PROTECT(allocMatrix(REALSXP, height, width)); /* 1 */
    mp->sg = PROTECT(allocMatrix(REALSXP, height, width)); /* 2 */
    mp->sb = PROTECT(allocMatrix(REALSXP, height, width)); /* 3 */
    mp->sa = PROTECT(allocMatrix(REALSXP, height, width)); /* 4 */
    mp->rr = REAL(mp->sr);
    mp->rg = REAL(mp->sg);
    mp->rb = REAL(mp->sb);
    mp->ra = REAL(mp->sa);

    mp->protected += 4;

    mp->inited = 1;
}

static void
fhw(im2k_p im2k, const char *file,
    unsigned int height, unsigned int width, unsigned int depth) {
    mytype_p mp = void2mytype(im2k->cookie);

    if (!mp->inited) {
        init(mp, height, width, depth);
    }
}


/*
 * the odd thing here is that we need to insert the pixel in its
 * "transposed" position.  this is to keep the R code from having to
 * convert it to a matrix using "byrow=TRUE".  i.e., all for the sake
 * of efficiency.
 */
static void
addpixel(im2k_p im2k, int i, float red, float green, float blue, float alpha) {
    mytype_p mp = void2mytype(im2k->cookie);
    int j;

    j = transposei(mp, i);

#if 0
    fprintf(stderr, "%d %f %f %f %f\n", i, red, green, blue, alpha);
#endif /* 0 */
    mp->rr[j] = red;
    mp->rg[j] = green;
    mp->rb[j] = blue;
    mp->ra[j] = alpha;
}


static void
getpixels(im2k_p im2k, int i,
          float *pred, float *pgreen, float *pblue, float *palpha) {
    mytype_p mp = void2mytype(im2k->cookie);
    int j;

    j = transposei(mp, i);

    *pred = mp->rr[j];
    *pgreen = mp->rg[j];
    *pblue = mp->rb[j];
    *palpha = 0xff;             /* XXX */
}


/*
 * from "Writing R Extensions" (in turn said to be based on a similar
 * routine in the package stats), though this is modified to deal with
 * pairlists.
 */

/* get the list element named str, or report an error() */
static SEXP
getPairListElement(SEXP pairs, const char *str, const char *cmd) {
    for (pairs = CDR(pairs); pairs != R_NilValue; pairs = CDR(pairs)) {
        if (!isNull(TAG(pairs))) {
            if (strcmp(CHAR(PRINTNAME(TAG(pairs))), str) == 0) {
                return CAR(pairs);
            }
        }
    }
    error("%s: required parameter '%s' not specified", cmd, str);
    /*NOTREACHED*/
}


/*
 * given the name of an image file, return some of its metadata and
 * its R, G, and B channels.
 */

/* usage: imageread(char *filename, bool usemagickwand); */

static SEXP
image2kread(SEXP args) {
    const char *file;
    SEXP rval, names;
    mytype_p mp;
    readfile_t readfile;
    int protected = 0;
    int usemagick;
    im2k_t im2k;

    /* first, get file name */
    file = CHAR(STRING_ELT(getPairListElement(args, "file", "imageread"), 0));

    /* second, get library to use */
    usemagick = LOGICAL(getPairListElement(args, "usemagickwand", "imageread"))[0];
    if (usemagick == NA_LOGICAL) {
        error("'usemagickwand' must be TRUE or FALSE");
    }

    /* now, which should we use, Imlib2 or MagicWand? */
#if defined(HAVE_IMLIB2) && defined(HAVE_MAGICKWAND)
    if (usemagick) {
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

    mp = mytypecreate();

    im2k.errprint = myerrprint;
    im2k.exit = myexit;
    im2k.malloc = mymalloc;
    im2k.cookie = mp;

    /* read in the image, filling in *mp as a side effect */
    (readfile)(&im2k, file, fhw, addpixel);

    // now, put together the return: height, width, r, g, b
    rval = PROTECT(list5(PROTECT(ScalarReal(mp->depth)), /* 1,2 */
                         mp->sr,
                         mp->sg,
                         mp->sb,
                         mp->sa));
    protected += 2;

    names = PROTECT(list5(PROTECT(mkString("depth")),   /* 1,2 */
                          PROTECT(mkString("red")),     /* 3 */
                          PROTECT(mkString("green")),   /* 4 */
                          PROTECT(mkString("blue")),    /* 5 */
                          PROTECT(mkString("alpha")))); /* 6 */
    protected += 6;
    namesgets(rval, names);

    protected += mp->protected;
    mytypedestroy(mp);

    UNPROTECT(protected);

    return rval;
}

/*
 * given the red, green, and blue channels of an image, and the
 * desired depth, write it out to the given file name.
 */
static SEXP
image2kwrite(SEXP args) {
    const char *file;
    mytype_p mp;
    int usemagick;
    int protected = 0;          /* for UNPROTECT */
    writefile_t writefile;
    im2k_t im2k;

    /* first, get file name */
    file = CHAR(STRING_ELT(getPairListElement(args, "file", "imageread"), 0));

    /* second, get library to use */
    usemagick = LOGICAL(getPairListElement(args, "usemagickwand", "imageread"))[0];
    if (usemagick == NA_LOGICAL) {
        error("'usemagickwand' must be TRUE or FALSE");
    }

    /* now, which should we use, Imlib2 or MagicWand? */
#if defined(HAVE_IMLIB2) && defined(HAVE_MAGICKWAND)
    if (usemagick) {
        writefile = writefilek;
    } else {
        writefile = writefile2;
    }
#elif defined(HAVE_IMLIB2)
    writefile = writefile2;
#elif defined(HAVE_MAGICKWAND)
    writefile = writefilek;
#else
// this should not occur!
#error Need Imlib2 or ImageMagick -- neither defined at compilation time
#endif /* defined(HAVE_IMLIB2) && defined(HAVE_MAGICKWAND) */
    mp = mytypecreate();

    mp->sr = getPairListElement(args, "red", "imagewrite");
    mp->sg = getPairListElement(args, "green", "rimatewrite");
    mp->sb = getPairListElement(args, "blue", "imagewrite");

    if ((!isMatrix(mp->sr)) || (!isMatrix(mp->sg)) || (!isMatrix(mp->sb))) {
        error("imagewrite: the red, green, and blue parameters must be matrices");
        /*NOTREACHED*/
    }

    if ((nrows(mp->sr) != nrows(mp->sg)) || (nrows(mp->sg) != nrows(mp->sb)) ||
        (ncols(mp->sr) != ncols(mp->sg)) || (ncols(mp->sg) != ncols(mp->sb))) {
        error("imagewrite: the red, green, and blue matrices must have the same dimensions");
        /*NOTREACHED*/
    }

    mp->hhh = nrows(mp->sr);
    mp->www = ncols(mp->sr);

    mp->depth = REAL(getPairListElement(args, "depth", "imagewrite"))[0];

    mp->rr = REAL(mp->sr);
    mp->rg = REAL(mp->sg);
    mp->rb = REAL(mp->sb);

    im2k.errprint = myerrprint;
    im2k.exit = myexit;
    im2k.malloc = mymalloc;
    im2k.cookie = mp;

    /* write out the image */
    (writefile)(&im2k, file, mp->hhh, mp->www, mp->depth, getpixels);

    protected += mp->protected;
    mytypedestroy(mp);

    UNPROTECT(protected);

    return R_NilValue;
}

/* R glue */

void R_init_image2k(DllInfo *info)
{
    static const R_ExternalMethodDef externalMethods[] = {
        {"image2kread",  (DL_FUNC) &image2kread, -1},
        {"image2kwrite",  (DL_FUNC) &image2kwrite, -1},
        {NULL}
    };
    /* Register the .External routine.
     * No .C, .Call, or Fortran routines,
     * so pass those arrays as NULL.
     */
    R_registerRoutines(info, NULL, NULL, NULL, externalMethods);
}
