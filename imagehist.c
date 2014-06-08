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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/errno.h>
#include <sys/stat.h>

// http://docs.enlightenment.org/api/imlib2/html/
#include <Imlib2.h>

#include "imageutils.h"


static unsigned int www, hhh, len, nfiles, arraysize;
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
init(void) {
    www = imlib_image_get_width();
    hhh = imlib_image_get_height();
    len = www*hhh;
    arraysize = IMAGE_NVALS*sizeof(long);
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
    nfiles = 0;
    inited = 1;
}

static void
chkcompat(char *file) {
    unsigned int w, h;

    w = imlib_image_get_width();
    h = imlib_image_get_height();

    if ((w != www) || (h != hhh)) {
        fprintf(stderr, "incompatible file \"%s\": (%d, %d) != (%d, %d)\n",
                file, www, hhh, w, h);
        exit(6);
        /*NOTREACHED*/
    }
}

static void
dofile(char *file) {
    Imlib_Image x;              /* imlib2 context */
    Imlib_Load_Error error_return;
    DATA32 *data;               /* actual image data */
    int i;
    unsigned int val;
    unsigned int r, g, b, l;

    x = imlib_load_image_with_error_return(file, &error_return);
    if (x == NULL) {
        fprintf(stderr, "unable to open \"%s\": %s\n",
                file, image_decode_load_error(error_return));
        exit(7);
        /*NOTREACHED*/
    }
    imlib_context_set_image(x);

    if (!inited) {
        init();
    } else {
        chkcompat(file);
    }

    data = imlib_image_get_data_for_reading_only();

    for (i = 0; i < len; i++) {
        val = data[i];
        r = GetR(val);
        g = GetG(val);
        b = GetB(val);
        l = PPM_ARGB_TO_LUM(val);
        rhist[r]++;
        ghist[g]++;
        bhist[b]++;
        lhist[l]++;
    }
    nfiles++;                   /* processed another file */
    imlib_free_image_and_decache();
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
    for (i = 0; i < IMAGE_NVALS; i++) {
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
    
    while ((ch = getopt(argc, argv, "h")) != -1) {
        switch (ch) {
        case 'h':
            hflag++;
            break;
        }
    }

    argc -= optind;
    argv += optind;

    if (argc < 1) {
        usage(cmd);
        /*NOTREACHED*/
    }
    while (argc > 0) {
        dofile(argv[0]);
        argv++;
        argc--;
    }
    done();
    return(0);
}
