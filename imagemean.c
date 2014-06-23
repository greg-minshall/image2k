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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/errno.h>
#include <sys/stat.h>

#include "config.h"

#if defined(HAVE_IMLIB2)
#include <Imlib2.h>
#endif /* defined(HAVE_IMLIB2) */

#if  defined(HAVE_MAGICKWAND)
#include <wand/MagickWand.h>
#endif /* defined(HAVE_MAGICKWAND) */


#include "imageutils.h"


static unsigned int www, hhh, len, nfiles;
static float *rmean = 0, *gmean = 0, *bmean = 0, *amean = 0;
static int inited = 0;
static char *oname = 0;

/* callouts from Imlib2 and ImageMagick code */
typedef void (*fhwcall_t)(char *file, unsigned int height, unsigned int width);
typedef void (*pixel_t)(int i, float red, float green, float blue, float alpha);

/* used for negotiating between Imlib2 and ImageMagick. */
typedef void (*dofile_t)(char *file, fhwcall_t dofhw, pixel_t dopix);
typedef void (*done_t)(void);


static void
usage(char *cmd) {
    fprintf(stderr, "usage: %s [-2|-k] -[f]o outfile infile1 [infile2 ...]\n", cmd);
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
fhw(char *file, unsigned int height, unsigned int width) {
    if (!inited) {
        init(height, width);
    } else {
        chkcompat(file, height, width);
    }
}



static void
addpixel(int i, float red, float green, float blue, float alpha) {
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


#if defined(HAVE_IMLIB2)
/*
 * process a file with imlib2
 */
static void
dofile2(char *file, fhwcall_t dofhw, pixel_t dopix) {
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

    (dofhw)(file, h, w);

    data = imlib_image_get_data_for_reading_only();

    for (i = 0; i < len; i++) {
        val = data[i];
        (dopix)(i, GetR(val)*1.0, GetG(val)*1.0, GetB(val)*1.0, GetA(val)*1.0);
    }
    imlib_free_image_and_decache();
}


/*
 * finish processing with imlib2
 */
static void
done2(void) {
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
#endif /* defined(HAVE_IMLIB2) */

#if defined(HAVE_MAGICKWAND)

static void
ThrowWandException(MagickWand *wand) {
    char *description;

    ExceptionType severity;

    description=MagickGetException(wand,&severity);
    (void) fprintf(stderr,"%s %s %lu %s\n",GetMagickModule(),description);
    description=(char *) MagickRelinquishMemory(description);
    exit(-1);
            }

/*
 * the ImageMagick bits are mostly copied from the sample program
 * here:
 * http://www.imagemagick.org/script/magick-wand.php
 */

static void
    dofilek(char *file, fhwcall_t dofhw, pixel_t dopix) {
    long y;
    MagickBooleanType status;
    MagickPixelPacket pixel;
    MagickWand *image_wand;
    PixelIterator *iterator;
    PixelWand **pixels;
    register long x;
    unsigned long width;
    int i = 0, h, w;

    /* Read an image. */
    MagickWandGenesis();
    image_wand = NewMagickWand();
    status = MagickReadImage(image_wand, file);
    if (status == MagickFalse) {
        ThrowWandException(image_wand);
    }
    h = MagickGetImageHeight(image_wand);
    w = MagickGetImageWidth(image_wand);

    (dofhw)(file, h, w);

    iterator = NewPixelIterator(image_wand);
    if (iterator == (PixelIterator *) NULL) {
        ThrowWandException(image_wand);
    }
    for (y=0; y < h; y++) {
        pixels = PixelGetNextIteratorRow(iterator,&width);
        if (pixels == (PixelWand **) NULL) {
            break;
        }
        for (x=0; x < (long) width; x++) {
            // PixelGet* returns in range [0,1); we like [0..255]
            (dopix)(i,
                    PixelGetRed(pixels[x])*255,
                    PixelGetGreen(pixels[x])*255,
                    PixelGetBlue(pixels[x])*255,
                    PixelGetAlpha(pixels[x])*255);
            i++;
        }
    }
    if (y < (long) MagickGetImageHeight(image_wand)) {
        ThrowWandException(image_wand);
    }
    iterator = DestroyPixelIterator(iterator);
    image_wand = DestroyMagickWand(image_wand);
}

// this bit is based on:
// http://members.shaw.ca/el.supremo/MagickWand/grayscale.htm

static void
donek() {
    MagickWand *m_wand = NULL;
    PixelWand *p_wand = NULL;
    PixelIterator *iterator = NULL;
    PixelWand **pixels = NULL;
    unsigned long x, y;
    int i;

    MagickWandGenesis();
    m_wand = NewMagickWand();
    p_wand = NewPixelWand();
    PixelSetColor(p_wand, "white");
    MagickNewImage(m_wand, www, hhh, p_wand);
    // Get a new pixel iterator 
    iterator = NewPixelIterator(m_wand);
    i = 0;
    for(y = 0; y < hhh; y++) {
        // Get the next row of the image as an array of PixelWands
        pixels = PixelGetNextIteratorRow(iterator, &x);
        // Set the row of wands to a simple gray scale gradient
        for(x = 0; x < www; x++) {
            PixelSetRed(pixels[x], rmean[i]/255.0);
            PixelSetGreen(pixels[x], gmean[i]/255.0);
            PixelSetBlue(pixels[x], bmean[i]/255.0);
            PixelSetAlpha(pixels[x], amean[i]/255.0);
            i++;
        }
        // Sync writes the pixels back to the m_wand
        PixelSyncIterator(iterator);
    }
    MagickWriteImage(m_wand, oname);
    // Clean up
    iterator = DestroyPixelIterator(iterator);
    DestroyMagickWand(m_wand);
    MagickWandTerminus();
}
#endif /* defined(HAVE_MAGICKWAND) */


int
main(int argc, char *argv[]) {
    int ch;
    char *cmd = argv[0];
    int force = 0;              /* overwrite file */
    struct stat statbuf;
    dofile_t dofile = dofile2;
    done_t done = done2;
    int flag2 = 0, flagk = 0;
    int flagx = 0;              /* undocumented, for internal testing */

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
        dofile = dofilek;
        if (!flagx) {
            done = donek;
        } else {
            done = done2;       /* mix and match (for debugging) */
        }
    } else {
        dofile = dofile2;
        if (!flagx) {
            done = done2;
        } else {
            done = donek;       /* again, mix and match (for debugging) */
        }
    }
#elif defined(HAVE_IMLIB2)
    dofile = dofile2;
    done = done2;
#elif defined(HAVE_MAGICKWAND)
    dofile = dofilek;
    done = donek;
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
    while (argc > 0) {
        (*dofile)(argv[0], fhw, addpixel);
        argv++;
        argc--;
        nfiles++;
    }
#ifdef HAVE_IMLIB2
    (*done)();
#endif /* def HAVE_IMLIB2 */
    return(0);
}
