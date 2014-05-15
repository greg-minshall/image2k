/*
 * given a list of input TIFF files, create an output TIFF file in
 * which each pixel represents the average the of pixel values of the
 * input files.
 */

#include <stdio.h>
#include <stdlib.h>

#include <tiffio.h>

static uint32 www, hhh, len, nfiles;
static uint32 *average = 0, uint32 *current = 0;
static int inited = 0;

static void
usage(char *cmd) {
    fprintf(stderr, "usage: %s file ...\n", cmd);
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
    TIFFGetField(x, TIFFTAG_IMAGEHEIGHT, &hhh);
    len = www*hhh;
    average = (uint32*) _TIFFmalloc(len*sizeof(uint32));
    current = (uint32*) _TIFFmalloc(len*sizeof(uint32));
    if ((average == NULL) || (current == NULL)) {
        perror("no room to initialize array");
        exit(5);
        /*NOTREACHED*/
    }
    nfiles = 0;
    inited = 1;
}

static void
chkcompat(TIFF *x, char *file) {
    uint 32 w, h;

    TIFFGetField(x, TIFFTAG_IMAGEWIDTH, &w);
    TIFFGetField(x, TIFFTAG_IMAGEHEIGHT, &h);

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
    unsigned int r0, r1, g0, g1, b0, b1, ret;
    int p = 0;

    x  = TIFFOpen(file, "r");
    if (x == NULL) {
        fprintf(stderr, "unable to open \"%s\": %s\n", file, strerror());
        exit(7);
        /*NOTREACHED*/
    }
    if (TIFFReadRGBAImage(x, www, hhh, current, 0) == 0) {
        fprintf(stderr, "error in TIFFReadRGBAImage for \"%s\"\n", file);
        exit(8);
        /*NOTREACHED*/
    }
    /* process TIFF */
    TIFFClose(x);
}

static void
done() {
    int p = 0;

    for (p = 0; p < NPIXELS; p++) {
        r[p] = r[p]/nfiles;
        g[p] = g[p]/nfiles;
        b[p] = b[p]/nfiles;
    }
}

int
main(int argc, char *argv[]) {
    if (argc < 2) {
        usage(argv[0]);
        /*NOTREACHED*/
    }
    argc--;
    argv++;
    while (argc > 0) {
        dofile(argv[0]);
        argv++;
        argc--;
    }
    done();
}
