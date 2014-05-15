#include <stdio.h>
#include <stdlib.h>

#define DUMP() fprintf(stderr, "nvals = %d, oavg16 = %d, val = %d, avg32 = %d, avg32/nvals = %d(%g), (val-oavg16)/nvals = %d(%g), oavg16+((val-oavg16)/nvals) = %d(%g), avg16 = %d\n", nvals, oavg16, val, avg32, avg32/nvals, (avg32*1.0)/(nvals*1.0), (val-oavg16)/nvals, ((val*1.0)-(oavg16*1.0))/(nvals*1.0), oavg16+((val-oavg16)/nvals), (oavg16*1.0)+(((val*1.0)-(oavg16*1.0))/(nvals*1.0)), avg16);

int
main() {
    int avg32 = 0, avg16 = 0, navg16, oavg16;
    int val, nvals = 0;

    while (scanf(" %x", &val) == 1) {
        nvals++;
        avg32 += val;
        oavg16 = avg16;
        if (nvals) {
            navg16 = (avg16 + ((val-avg16)/nvals))&0xffff;
            /* fix up low order */
            if ((((val-avg16)/nvals)*nvals) != (val-avg16)) {
                if (val-avg16 > 0) {
                } else {
                    navg16--;
                }
            }
            avg16 = navg16;
        } else {
            avg16 = val&0xffff;
        }
        if (avg16&0x10000) {
            fprintf(stderr, "overflow: ");
            DUMP();
            exit(1);
        }
        if (avg16 != (avg32/nvals)) {
            fprintf(stderr, "discrepancy: ");
            DUMP();
            exit(2);
        }
        DUMP();
    }
}
