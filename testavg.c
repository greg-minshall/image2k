#include <stdio.h>
#include <stdlib.h>

#define DUMP() fprintf(stderr, "nvals = %d, avg32 = %d, avg32/nvals = %d, avg16 = %d, val = %d\n", nvals, avg32, avg32/nvals, avg16, val);

int
main() {
    int avg32 = 0, avg16 = 0;
    int val, nvals = 0;

    while (scanf(" %x", &val) == 1) {
        nvals++;
        avg32 += val;
        if (nvals) {
            avg16 = (avg16 + ((val-avg16)/nvals))&0xffff;
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
