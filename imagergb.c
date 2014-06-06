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

#include <Imlib2.h>

#define GetR(abgr) (((abgr)    )&0xff)
#define GetG(abgr) (((abgr)>> 8)&0xff)
#define GetB(abgr) (((abgr)>>16)&0xff)
#define GetA(abgr) (((abgr)>>24)&0xff)

// from netpbm/ppm.h
#define PPM_LUMINR 0.2989
#define PPM_LUMING 0.5866
#define PPM_LUMINB 0.1145

#define PPM_ABGR_TO_LUM(abgr) \
    ((GetR(abgr)*PPM_LUMINR)+(GetG(abgr)*PPM_LUMING)+(GetB(abgr)*PPM_LUMINB))
//

static unsigned int www, hhh, len, nfiles;
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
init(void) {
    www = imlib_image_get_width();
    hhh = imlib_image_get_height();
    len = www*hhh;
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
    Imlib_Image image;              /* imlib2 context */
    DATA32 *data;               /* actual image data */
    int i, val;
    int r, g, b, a, l, x, y;

    image  = imlib_load_image_without_cache(file);
    if (image == NULL) {
        fprintf(stderr, "unable to open \"%s\": %s\n", file, strerror(errno));
        exit(7);
        /*NOTREACHED*/
    }
    imlib_context_set_image(image);

    if (!inited) {
        init();
    } else {
        chkcompat(file);
    }

    data = imlib_image_get_data_for_reading_only();

    for (i = 0; i < len; i++) {
        if (cvalue) {           /* print out x and y coordinates */
            y = i/www;
            x = i-(y*www);
            printf("%d %d ", x, y);
        }
        val = data[i];
        r = GetR(val);
        g = GetG(val);
        b = GetB(val);
        printf("%d %d %d", r, g, b);
        if (avalue) {           /* print out alpha value */
            a = GetA(val);
            printf(" %d", a);
        }
        if (lvalue) {           /* (compute and) print out luminance value */
            l = PPM_ABGR_TO_LUM(val);
            printf(" %d", l);
        }
        printf("\n");
    }
    nfiles++;                   /* processed another file */
    imlib_free_image_and_decache();
}

static void
done() {
}

int
main(int argc, char *argv[]) {
    int ch;
    char *cmd = argv[0];
    int force = 0;              /* overwrite file */
    struct stat statbuf;
    
    while ((ch = getopt(argc, argv, "acl")) != -1) {
        switch (ch) {
        case 'a':
            avalue = 1;
            break;
        case 'c':
            cvalue = 1;
            break;
        case 'l':
            lvalue = 1;
            break;
        default:
            usage(cmd);
            /*NOTREACHED*/
        }
    }

    argc -= optind;
    argv += optind;

    if (argc != 1) {
        usage(cmd);
        /*NOTREACHED*/
    }

    dofile(argv[0]);
    done();
    return(0);
}
