/*
 * given a list of input TIFF files, create an output TIFF file in
 * which each pixel represents the average the of pixel values of the
 * input files.
 */

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

#include <tiffio.h>

/* it seems like these *should* be in tiffio.h */
#define TIFFPutR(abgr, r) (((abgr)&0xffffff00) | (((r)    )&0x000000ff))
#define TIFFPutG(abgr, g) (((abgr)&0xffff00ff) | (((g)<< 8)&0x0000ff00))
#define TIFFPutB(abgr, b) (((abgr)&0xff00ffff) | (((b)<<16)&0x00ff0000))
#define TIFFPutA(abgr, a) (((abgr)&0x00ffffff) | (((a)<<24)&0xff000000))

static uint32 www, hhh, len, nfiles;
static uint32 *current = 0;
static float *rmean = 0, *gmean = 0, *bmean = 0, *amean = 0;
static int inited = 0;
static FILE *ofile = NULL;

static void
usage(char *cmd) {
    fprintf(stderr, "usage: %s -o outfile infile ...\n", cmd);
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
static uint32
ravg(int n, uint32 cur, uint32 new)
{
    return cur+((new-cur)/n);
}

/*
 * initialize data structures
 */

static void
init(TIFF *x) {
    TIFFGetField(x, TIFFTAG_IMAGEWIDTH, &www);
    TIFFGetField(x, TIFFTAG_IMAGELENGTH, &hhh);
    len = www*hhh;
    rmean = (float*) _TIFFmalloc(len*sizeof(float));
    gmean = (float*) _TIFFmalloc(len*sizeof(float));
    bmean = (float*) _TIFFmalloc(len*sizeof(float));
    amean = (float*) _TIFFmalloc(len*sizeof(float));
    current = (uint32*) _TIFFmalloc(len*sizeof(uint32));
    if ((rmean == NULL) || (gmean == NULL) || (bmean == NULL) || (amean == NULL) ||
        (current == NULL)) {
        perror("no room to initialize array");
        exit(5);
        /*NOTREACHED*/
    }
    nfiles = 0;
    inited = 1;
}

static void
chkcompat(TIFF *x, char *file) {
    uint32 w, h;

    TIFFGetField(x, TIFFTAG_IMAGEWIDTH, &w);
    TIFFGetField(x, TIFFTAG_IMAGELENGTH, &h);

    if ((w != www) || (h != hhh)) {
        fprintf(stderr, "incompatible file \"%s\": (%d, %d) != (%d, %d)\n",
                file, www, hhh, w, h);
        exit(6);
        /*NOTREACHED*/
    }
}

static void
dofile(char *file) {
    TIFF *x;
    int i, val;

    x  = TIFFOpen(file, "r");
    if (x == NULL) {
        fprintf(stderr, "unable to open \"%s\": %s\n", file, strerror(errno));
        exit(7);
        /*NOTREACHED*/
    }

    if (!inited) {
        init(x);
    } else {
        chkcompat(x, file);
    }

    if (TIFFReadRGBAImage(x, www, hhh, current, 0) == 0) {
        fprintf(stderr, "error in TIFFReadRGBAImage for \"%s\"\n", file);
        exit(8);
        /*NOTREACHED*/
    }
    if (nfiles == 0) {          /* if this is first file */
        for (i = 0; i < len; i++) {
            val = current[i];
            rmean[i] = TIFFGetR(val)*1.0;
            gmean[i] = TIFFGetG(val)*1.0;
            bmean[i] = TIFFGetB(val)*1.0;
            amean[i] = TIFFGetA(val)*1.0;
        }
    } else {                    /* else, read into current and process */
        for (i = 0; i < len; i++) {
            /* process TIFF */
            val = current[i];
            rmean[i] += (((TIFFGetR(val)*1.0)-rmean[i])/(nfiles*1.0));
            gmean[i] += (((TIFFGetG(val)*1.0)-gmean[i])/(nfiles*1.0));
            bmean[i] += (((TIFFGetB(val)*1.0)-bmean[i])/(nfiles*1.0));
            amean[i] += (((TIFFGetA(val)*1.0)-amean[i])/(nfiles*1.0));
        }
    }
    nfiles++;                   /* processed another file */
    TIFFClose(x);
}

static void
done() {
}

int
main(int argc, char *argv[]) {
    int ch;
    char *cmd = argv[0];
    char *oname;
    struct stat statbuf;
    
    while ((ch = getopt(argc, argv, "o:")) != -1) {
        switch (ch) {
        case 'o':
            oname = optarg;
            if ((stat(oname, &statbuf) != -1) || (errno != ENOENT)) {
                fprintf(stderr, "file \"%s\" exists, not overwritten\n", oname);
                exit(1);
            }
            ofile = fopen(oname, "w");
            if (ofile == NULL) {
                fprintf(stderr, "unable to open \"%s\" for writing: ", oname);
                perror("fopen");
                exit(2);
            }
        }
    }

    argc -= optind;
    argv += optind;

    if ((argc < 1) || (ofile == NULL)) {
        usage(cmd);
        /*NOTREACHED*/
    }
    while (argc > 0) {
        dofile(argv[0]);
        argv++;
        argc--;
    }
    done();
}
