/*
 * convert an image to its rgb(a) values
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/errno.h>
#include <sys/stat.h>

#include "config.h"

#include "image2k.h"

static unsigned int www, hhh, len, depth;
static int inited = 0;

static unsigned int avalue = 0; /* output alpha channel */
static unsigned int cvalue = 0; /* output x and y coordinates (as first two columns)  */
static unsigned int lvalue = 0; /* output computed luminance */

static void
usage(char *cmd) {
    fprintf(stderr, "usage: %s [-al] infile\n", cmd);
    exit(1);
}


/*
 * initialize data structures
 */

static void
init(unsigned int height, unsigned int width, unsigned int passed_depth) {
    hhh = height;
    www = width;
    depth = passed_depth;
    len = www*hhh;
    inited = 1;
}

static void
chkcompat(const char *file,
          unsigned int height, unsigned int width, unsigned int depth) {
    if ((width != www) || (height != hhh)) {
        fprintf(stderr, "incompatible file \"%s\": (%d, %d) != (%d, %d)\n",
                file, www, hhh, width, height);
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
output(im2k_p im2k, int i, float fr, float fg, float fb, float fa) {
    unsigned int r, g, b, a;
    unsigned int l;

    r = fr*255.0;
    g = fg*255.0;
    b = fb*255.0;
    a = fa*255.0;

    printf("%d %d %d", r, g, b);
    if (avalue) {           /* print out alpha value */
        printf(" %d", a);
    }
    if (lvalue) {           /* (compute and) print out luminance value */
        l = PPM_RGB_TO_LUM(r, g, b);
        printf(" %d", l);
    }
    printf("\n");
}


int
main(int argc, char *argv[]) {
    int ch;
    char *cmd = argv[0];
    int force = 0;              /* overwrite file */
    struct stat statbuf;
    readfile_t readfile;
    int flag2 = 0, flagk = 0;
    im2k_t im2k;
    
    while ((ch = getopt(argc, argv, "2ackl")) != -1) {
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
        case 'a':
            avalue = 1;
            break;
        case 'c':
            cvalue = 1;
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
        case 'l':
            lvalue = 1;
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

    if (argc != 1) {
        usage(cmd);
        /*NOTREACHED*/
    }

    im2k.fprintf = fprintf;
    im2k.exit = exit;
    im2k.malloc = malloc;
    im2k.cookie = 0;

    (readfile)(&im2k, argv[0], fhw, output);
    return(0);
}
