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

/*
 * the ImageMagick bits are mostly copied from the sample program
 * here: http://www.imagemagick.org/script/magick-wand.php
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/errno.h>
#include <sys/stat.h>

#include "config.h"

#ifdef HAVE_IMLIB2
#include <Imlib2.h>
#endif /* def HAVE_IMLIB2 */

#ifdef HAVE_IMAGEMAGICK
#include <wand/MagickWand.h>
#endif /* def HAVE_IMAGEMAGICK */

#include "imageutils.h"


static unsigned int www, hhh, len, nfiles;
static float *rmean = 0, *gmean = 0, *bmean = 0, *amean = 0;
static int inited = 0;
static char *oname = 0;

static void
usage(char *cmd) {
    fprintf(stderr, "usage: %s -[f]o outfile infile1 [infile2 ...]\n", cmd);
    exit(1);
}

static int
endian(int byte0, int byte1) {
    return (byte1*0x100)+byte0; /* XXX little endian */
}


/*
 * we've calculated the running average CUR of N samples, and now
 * compute the running average taking into account the sample NEW.
 */
static unsigned int
ravg(int n, unsigned int cur, unsigned int new)
{
    return cur+((new-cur)/n);
}


/*
 * initialize data structures
 */

static void
init(unsigned int height, unsigned int width) {
    hhh = height;
    www = width;
    len = www*hhh;
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
chkcompat(char *file, unsigned int height, unsigned int width) {
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
addpixel(int i, float red, float green, float blue, float alpha) {
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
im2dofile(char *file) {
    Imlib_Image x;              /* imlib2 context */
    DATA32 *data;               /* actual image data */
    int i, val, w, h;

    x  = imlib_load_image_without_cache(file);
    if (x == NULL) {
        Imlib_Load_Error error_return;
        x = imlib_load_image_with_error_return(file, &error_return);
        fprintf(stderr, "unable to open \"%s\": %s\n",
                file, image_decode_load_error(error_return));
        exit(7);
        /*NOTREACHED*/
    }
    imlib_context_set_image(x);

    w = imlib_image_get_width();
    h = imlib_image_get_height();

    if (!inited) {
        init(h, w);
    } else {
        chkcompat(file, h, w);
    }

    data = imlib_image_get_data_for_reading_only();

    for (i = 0; i < len; i++) {
        val = data[i];
        addpixel(i, GetR(val)*1.0, GetG(val)*1.0, GetB(val)*1.0, GetA(val)*1.0);
    }
    imlib_free_image_and_decache();
}

static void
im2done() {
    int i, r, g, b, a, val;
    Imlib_Image outimage;
    DATA32 *outdata;
    Imlib_Load_Error imerr;

    outdata = (DATA32 *)malloc(len*(sizeof (DATA32)));
    if (outdata == NULL) {
        fprintf(stderr, "no room for output buffer\n");
        exit(9);
    }

    for (i = 0; i < len; i++) {
        val = 0;
        r = rmean[i];
        g = gmean[i];
        b = bmean[i];
        a = amean[i];
        val |= PutR(val, r);
        val |= PutG(val, g);
        val |= PutB(val, b);
        val |= PutA(val, a);
        outdata[i] = val;
    }
    outimage = imlib_create_image_using_data(www, hhh, outdata);
    if (outimage == NULL) {
        fprintf(stderr, "unable to create output image structures (internal): ");
        perror("");
        exit(10);
    }
    imlib_context_set_image(outimage);    
    imlib_save_image_with_error_return(oname, &imerr);
    if (imerr != IMLIB_LOAD_ERROR_NONE) {
        fprintf(stderr, "error saving output file \"%s\": ", oname);
        perror("");
        exit(10);
    }
    imlib_free_image_and_decache();
}

int
main(int argc, char *argv[]) {
    int ch;
    char *cmd = argv[0];
    int force = 0;              /* overwrite file */
    struct stat statbuf;
    
    while ((ch = getopt(argc, argv, "fo:")) != -1) {
        switch (ch) {
        case 'o':
            oname = optarg;
            break;
        case 'f':
            force = 1;
            break;
        default:
            usage(cmd);
            /*NOTREACHED*/
        }
    }

    argc -= optind;
    argv += optind;

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
    while (argc > 0) {
        im2dofile(argv[0]);
        argv++;
        argc--;
        nfiles++;
    }
    im2done();
    return(0);
}
