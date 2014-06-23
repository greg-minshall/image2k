#ifndef __image_h__
#define __image_h__

/* callouts from Imlib2 and ImageMagick code */
typedef void (*fhwcall_t)(char *file, unsigned int height, unsigned int width);
typedef void (*process_t)(int i, float red, float green, float blue, float alpha);
typedef void (*getpixels_t)(int i,
                            float *red, float *green, float *blue, float *alpha);

/* used for negotiating between Imlib2 and ImageMagick. */
typedef void (*dofile_t)(char *file, fhwcall_t dofhw, process_t dopix);
typedef void (*done_t)(char *ofile,
                       unsigned int hhh,
                       unsigned int www,
                       getpixels_t getpixels);


#if defined(HAVE_IMLIB2)
/*
 * These are for use with Imlib2.h, which keeps its data (as returned
 * from imlib_image_get_data() and friends) in this format:

"The image data is returned in the format of a DATA32 (32 bits) per
pixel in a linear array ordered from the top left of the image to the
bottom right going from left to right each line. Each pixel has the
upper 8 bits as the alpha channel and the lower 8 bits are the blue
channel - so a pixel's bits are ARGB (from most to least significant,
8 bits per channel)."

 * (from the documentation for imlib_image_get_data())
 */

#define GetA(argb) (((argb)>>24)&0xff)
#define GetR(argb) (((argb)>>16)&0xff)
#define GetG(argb) (((argb)>> 8)&0xff)
#define GetB(argb) (((argb)    )&0xff)

#define PutA(argb, a) (((argb)&0x00ffffff) | (((a)<<24)&0xff000000))
#define PutR(argb, b) (((argb)&0xff00ffff) | (((b)<<16)&0x00ff0000))
#define PutG(argb, g) (((argb)&0xffff00ff) | (((g)<< 8)&0x0000ff00))
#define PutB(argb, r) (((argb)&0xffffff00) | (((r)    )&0x000000ff))

// how to compute luminance
// from netpbm/ppm.h
#define PPM_LUMINR 0.2989
#define PPM_LUMING 0.5866
#define PPM_LUMINB 0.1145

#define PPM_ARGB_TO_LUM(argb) \
    ((GetR(argb)*PPM_LUMINR)+(GetG(argb)*PPM_LUMING)+(GetB(argb)*PPM_LUMINB))

#define IMAGE_NVALS 256         /* number of possible values in an image */

/*
 * process a file with imlib2
 */
void dofile2(char *file, fhwcall_t dofhw, process_t dopix);
void done2(char *ofile, unsigned int hhh, unsigned int www, getpixels_t getpixels);
#endif /* defined(HAVE_IMLIB2) */

/*
 * process file with imagemagick
 */
void dofilek(char *file, fhwcall_t dofhw, process_t dopix);
void donek(char *ofile, unsigned int hhh, unsigned int www, getpixels_t getpixels);

#endif /* ndef __image_h__ */
