/*
 * given a list of input images, create a histogram of the red, green,
 * blue, and (computed) luminance values in the inputs.
 */

/* $Id$ */

/*
 * so, an image:

L1001611.tif TIFF 5976x3992 5976x3992+0+0 16-bit sRGB 143.2MB 0.000u 0:00.009
 
 * has 143137152 data bytes, which is 5976*3992*6 == 23856192*6, i.e.,
 * there are 2 bytes (16-bits) for each of R, G, and B.
 */


#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/errno.h>
#include <sys/stat.h>

#include "config.h"

#include "imageutils.h"


static unsigned int www, hhh, len, arraysize, depth, image_nvals;
static long *rhist, *ghist, *ahist, *bhist, *lhist;


static int inited = 0;
static int hflag = 0;

static void
usage(char *cmd) {
    fprintf(stderr, "usage: %s infile1 [infile2 ...]\n", cmd);
    exit(1);
}

static int
endian(int byte0, int byte1) {
    return (byte1*0x100)+byte0; /* XXX little endian */
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
    image_nvals = exp2(depth);

    arraysize = image_nvals*sizeof(long);

    rhist = (long*) malloc(arraysize);
    ghist = (long*) malloc(arraysize);
    bhist = (long*) malloc(arraysize);
    ahist = (long*) malloc(arraysize);
    lhist = (long*) malloc(arraysize);
    if ((rhist == NULL) || (ghist == NULL) ||
        (bhist == NULL) || (ahist == NULL) || (lhist == NULL)) {
        perror("no room to initialize arrays");
        exit(5);
        /*NOTREACHED*/
    }
    bzero(rhist, arraysize);
    bzero(ghist, arraysize);
    bzero(bhist, arraysize);
    bzero(ahist, arraysize);
    bzero(lhist, arraysize);
    inited = 1;
}


static void
chkcompat(char *file, unsigned int height, unsigned int width, unsigned int depth) {
    if ((width != www) || (height != hhh)) {
        fprintf(stderr, "incompatible file \"%s\": (%d, %d) != (%d, %d)\n",
                file, www, hhh, width, height);
        exit(6);
        /*NOTREACHED*/
    }
}

static void
fhw(char *file, unsigned int height, unsigned int width, unsigned int depth) {
    if (!inited) {
        init(height, width, depth);
    } else {
        chkcompat(file, height, width, depth);
    }
}


static void
addhist(int i, float fr, float fg, float fb, float fa) {
    unsigned int r, g, b, a;
    unsigned int l;

    r = fr*255.0;
    g = fg*255.0;
    b = fb*255.0;

    l = PPM_RGB_TO_LUM(r, g, b);
    rhist[r]++;
    ghist[g]++;
    bhist[b]++;
    lhist[l]++;
}

static void
done() {
    int i, r, g, b, a, val;

    if (hflag) {                /* print out a header */
        printf("value rcount gcount bcount acount lcount\n");
        if (hflag > 1) {
            printf("----------------------------------------\n");
        }
    }
    for (i = 0; i < image_nvals; i++) {
        if (rhist[i] || ghist[i] || bhist[i] || ahist[i]) {
            printf("%d %ld %ld %ld %ld %ld\n",
                   i, rhist[i], ghist[i], bhist[i], ahist[i], lhist[i]);
        }
    }
}

int
main(int argc, char *argv[]) {
    int ch;
    char *cmd = argv[0];
    int force = 0;              /* overwrite file */
    struct stat statbuf;
    readfile_t readfile = readfile2;
    int flag2 = 0, flagk = 0;
    
    while ((ch = getopt(argc, argv, "2hk")) != -1) {
        switch (ch) {
        case '2':
#if defined(HAVE_IMLIB2)
            flag2 = 1;
#else /* defined(HAVE_IMLIB2) */
            fprintf(stderr, "%s -2: Imlib2 support not compiled in.\n", cmd);
            usage(cmd);
            /*NOTREACHED*/
#endif /* defined(HAVE_IMLIB2) */
            break;
        case 'h':
            hflag++;
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

    if (argc < 1) {
        usage(cmd);
        /*NOTREACHED*/
    }
    while (argc > 0) {
        (readfile)(argv[0], fhw, addhist);
        argv++;
        argc--;
    }
    done();
    return(0);
}
