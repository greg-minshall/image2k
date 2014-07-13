/*
 * given a list of input images, create an output image file in
 * which each pixel represents the average the of pixel values of the
 * input files.
 */

/* $Id$ */

/*
 * so, an image:

L1001611.tif TIFF 5976x3992 5976x3992+0+0 16-bit sRGB 143.2MB 0.000u 0:00.009
 
 * has 143137152 data bytes, which is 5976*3992*6 == 23856192*6, i.e.,
 * there are 2 bytes (16-bits) for each of R, G, and B.
 */

/*
 * this produced extracted bits (and one montage: m-10-49.png):
 *
 * % for i in `ls ~/Downloads/tmp/sensorproblem/ | sed s/.tif//g`; do     \
 *       convert -extract 137x158+1000+3800                             \
 *         ~/Downloads/tmp/sensorproblem/$i.tif clips/$i.png; done
 * % montage clips/L10001{0,1,2,3,4}?.png x.png
 *
 * and, how to use imagemean.c to produce an image containing the
 * average of each pixel:
 *
 * % ./a.out -o 130-139.png ~/Downloads/tmp/sensorproblem/L100013?.tif
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/errno.h>
#include <sys/stat.h>

#include "config.h"

#include "image2k.h"


static unsigned int www, hhh, len, nfiles, depth;
static float *rmean = 0, *gmean = 0, *bmean = 0, *amean = 0;
static int inited = 0;
static char *oname = 0;


static void
usage(char *cmd) {
    fprintf(stderr, "usage: %s [-2|-k] -[f]o outfile infile1 [infile2 ...]\n", cmd);
    exit(1);
}


/*
 * print for image2k routines
 */

static int
myerrprint(const char *restrict format, ...) {
    va_list ap;

    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);
    return(0);                  /* we can't [easily] match the spec... */
}




/*
 * initialize data structures
 */

static void
init(unsigned int height, unsigned int width, unsigned int passed_depth) {
    hhh = height;
    www = width;
    len = www*hhh;
    depth = passed_depth;

    rmean = (float*) malloc(len*sizeof(float));
    gmean = (float*) malloc(len*sizeof(float));
    bmean = (float*) malloc(len*sizeof(float));
    amean = (float*) malloc(len*sizeof(float));
    if ((rmean == NULL) || (gmean == NULL) || (bmean == NULL) || (amean == NULL)) {
        perror("no room to initialize arrays");
        exit(5);
        /*NOTREACHED*/
    }
    nfiles = 0;
    inited = 1;
}

static void
chkcompat(const char *file,
          unsigned int height, unsigned int width, unsigned int depth) {
    unsigned int w, h;

    h = height;
    w = width;

    if ((w != www) || (h != hhh)) {
        fprintf(stderr, "incompatible file \"%s\": (%d, %d) != (%d, %d)\n",
                file, www, hhh, w, h);
        exit(6);
        /*NOTREACHED*/
    }
}


static void
fhw(im2k_p im2k, const char *file,
    unsigned int height, unsigned int width, unsigned int depth) {
    if (!inited) {
        init(height, width, depth);
    } else {
        chkcompat(file, height, width, depth);
    }
}



static void
addpixel(im2k_p im2k, int i, float red, float green, float blue, float alpha) {
#if 0
    fprintf(stderr, "%d %f %f %f %f\n", i, red, green, blue, alpha);
#endif /* 0 */
    if (nfiles == 0) {          /* this is the first value */
        rmean[i] = red;
        gmean[i] = green;
        bmean[i] = blue;
        amean[i] = alpha;
    } else {
        /* process image data */
        rmean[i] += ((red-rmean[i])/(nfiles*1.0));
        gmean[i] += ((green-gmean[i])/(nfiles*1.0));
        bmean[i] += ((blue-bmean[i])/(nfiles*1.0));
        amean[i] += ((alpha-amean[i])/(nfiles*1.0));
    }
}

static void
getpixels(im2k_p im2k, int i,
          float *pred, float *pgreen, float *pblue, float *palpha) {
    *pred = rmean[i];
    *pgreen = gmean[i];
    *pblue = bmean[i];
    *palpha = amean[i];
}




int
main(int argc, char *argv[]) {
    int ch;
    char *cmd = argv[0];
    int force = 0;              /* overwrite file */
    struct stat statbuf;
    readfile_t readfile;
    writefile_t writefile;
    int flag2 = 0, flagk = 0;
    int flagx = 0;              /* undocumented, for internal testing */
    im2k_t im2k;

    while ((ch = getopt(argc, argv, "2fko:x")) != -1) {
        switch (ch) {
        case '2':               /* use Imlib2 */
#if defined(HAVE_IMLIB2)
            flag2 = 1;
#else /* defined(HAVE_IMLIB2) */
            fprintf(stderr, "%s -2: Imlib2 support not compiled in.\n", cmd);
            usage(cmd);
            /*NOTREACHED*/
#endif /* defined(HAVE_IMLIB2) */
            break;
        case 'f':
            force = 1;
            break;
        case 'k':               /* use ImageMagick */
#if defined(HAVE_MAGICKWAND)
            flagk = 1;
#else /* defined(HAVE_MAGICKWAND) */
            fprintf(stderr, "%s -2: Imlib2 support not compiled in.\n", cmd);
            usage(cmd);
            /*NOTREACHED*/
#endif /* defined(HAVE_MAGICKWAND) */
            break;
        case 'o':
            oname = optarg;
            break;
        case 'x':
#if !defined(HAVE_IMLIB2)
            fprintf(stderr, "%s -x: Imlib2 support not compiled in.\n", cmd);
            usage(cmd);
            /*NOTREACHED*/
#elif  !defined(HAVE_MAGICKWAND)
            fprintf(stderr, "%s -x: MagickWand support not compiled in.\n", cmd);
            usage(cmd);
            /*NOTREACHED*/
#else
            flagx = 1;
#endif
            break;
        default:
            usage(cmd);
            /*NOTREACHED*/
        }
    }

    if (flag2 && flagk) {
        fprintf(stderr, "%s: can't specify both -2 and -k\n", cmd);
        usage(cmd);
        /*NOTREACHED*/
    }

    argc -= optind;
    argv += optind;

    /* now, arbitrate between Imlib2 and ImageMagick */
#if defined(HAVE_IMLIB2) && defined(HAVE_MAGICKWAND)
    if (flagk) {
        readfile = readfilek;
        if (!flagx) {
            writefile = writefilek;
        } else {
            writefile = writefile2;       /* mix and match (for debugging) */
        }
    } else {
        readfile = readfile2;
        if (!flagx) {
            writefile = writefile2;
        } else {
            writefile = writefilek;       /* again, mix and match (for debugging) */
        }
    }
#elif defined(HAVE_IMLIB2)
    readfile = readfile2;
    writefile = writefile2;
#elif defined(HAVE_MAGICKWAND)
    readfile = readfilek;
    writefile = writefilek;
#else
// this should not occur!
#error Need Imlib2 or ImageMagick -- neither defined at compilation time
#endif /* defined(HAVE_IMLIB2) && defined(HAVE_MAGICKWAND) */

    if ((argc < 1) || (oname == NULL)) {
        usage(cmd);
        /*NOTREACHED*/
    }
    if ((stat(oname, &statbuf) != -1) || (errno != ENOENT)) {
        if (!force) {
            fprintf(stderr, "file \"%s\" exists, not overwritten\n", oname);
            exit(1);
        }
    }

    im2k.errprint = myerrprint;
    im2k.exit = exit;
    im2k.malloc = malloc;
    im2k.cookie = 0;
    while (argc > 0) {
        (*readfile)(&im2k, argv[0], fhw, addpixel);
        argv++;
        argc--;
        nfiles++;
    }
#ifdef HAVE_IMLIB2
    (*writefile)(&im2k, oname, hhh, www, depth, getpixels);
#endif /* def HAVE_IMLIB2 */
    return(0);
}
