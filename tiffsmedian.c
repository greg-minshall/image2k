#include <stdio.h>
#include <stdlib.h>

#define NPIXELS (5976*3992)
static int r[NPIXELS], g[NPIXELS], b[NPIXELS];
static int nfiles = 0;

static void
usage(char *cmd) {
    fprintf(stderr, "usage: %s file ...\n", cmd);
    exit(1);
}

static int
endian(int byte0, int byte1) {
    return (byte1*0x100)+byte0; /* XXX little endian */
}

static void
dofile(char *file) {
    FILE *x;
    unsigned int r0, r1, g0, g1, b0, b1, ret;
    int p = 0;

    x = fopen(file, "r");
    if (x == NULL) {
        perror(file);
        exit(2);
    }
    nfiles++;

    while ((ret = fscanf(x, " %02x %02x %02x %02x %02x %02x",
                         &r0, &r1, &g0, &g1, &b0, &b1)) == 6) {
        /* XXX little endian */
        if (p > NPIXELS) {
            fprintf(stderr, "exceeded array bounds\n");
            exit(3);
        }
        r[p] += endian(r0, r1);
        g[p] += endian(g0, g1);
        b[p] += endian(b0, b1);
        p++;
    }
    if (p != NPIXELS) {
        fprintf(stderr, "short file (got %d, expected %d); ret = %d\n",
                p, NPIXELS, ret);
        exit(4);
    }
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
    }
    done();
}
