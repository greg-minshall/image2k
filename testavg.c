#include <stdio.h>
#include <stdlib.h>

#define DUMP() fprintf(stderr, "nvals = %d, oavg16 = %d, val = %d, avg32 = %d, rnd(avg32/nvals) = %d(%g), (val-oavg16)/nvals = %d(%g), oavg16+((val-oavg16)/nvals) = %d(%g), avg16 = %d\n", nvals, oavg16, val, avg32, (avg32+(nvals/2))/nvals, (avg32*1.0)/(nvals*1.0), (val-oavg16)/nvals, ((val*1.0)-(oavg16*1.0))/(nvals*1.0), oavg16+((val-oavg16)/nvals), (oavg16*1.0)+(((val*1.0)-(oavg16*1.0))/(nvals*1.0)), avg16);

int
main() {
    int avg32 = 0, avg16 = 0, oavg16;
    int val, nvals = 0, tmp;

    while (scanf(" %x", &val) == 1) {
        nvals++;
        avg32 += val;
        oavg16 = avg16;
        if (nvals) {
            avg16 = (oavg16 + ((val-oavg16)/nvals))&0xffff;
            /* fix up low order */
            tmp = (((val-oavg16)/nvals)*nvals)-(val-oavg16);
            if (tmp != 0) {
                fprintf(stderr, "tmp = %d\n", tmp);
                if (abs(tmp) > (nvals/2)) {
                    if (val-oavg16 > 0) {
                        avg16++;
                    } else {
                        avg16--;
                    }
                }
            }
        } else {
            avg16 = val&0xffff;
        }
        if (avg16&0x10000) {
            fprintf(stderr, "overflow: ");
            DUMP();
            exit(1);
        }
        if (avg16 != ((avg32+(nvals/2))/nvals)) {
            fprintf(stderr, "discrepancy: ");
            DUMP();
            exit(2);
        }
        DUMP();
    }
}
