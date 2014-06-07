#ifndef __image_h__
#define __image_h__

#define GetR(abgr) (((abgr)    )&0xff)
#define GetG(abgr) (((abgr)>> 8)&0xff)
#define GetB(abgr) (((abgr)>>16)&0xff)
#define GetA(abgr) (((abgr)>>24)&0xff)

#define PutR(abgr, r) (((abgr)&0xffffff00) | (((r)    )&0x000000ff))
#define PutG(abgr, g) (((abgr)&0xffff00ff) | (((g)<< 8)&0x0000ff00))
#define PutB(abgr, b) (((abgr)&0xff00ffff) | (((b)<<16)&0x00ff0000))
#define PutA(abgr, a) (((abgr)&0x00ffffff) | (((a)<<24)&0xff000000))

// how to compute luminance
// from netpbm/ppm.h
#define PPM_LUMINR 0.2989
#define PPM_LUMING 0.5866
#define PPM_LUMINB 0.1145

#define PPM_ABGR_TO_LUM(abgr) \
    ((GetR(abgr)*PPM_LUMINR)+(GetG(abgr)*PPM_LUMING)+(GetB(abgr)*PPM_LUMINB))
//


#define IMAGE_NVALS 256         /* number of possible values in an image */

#endif /* ndef __image_h__ */
