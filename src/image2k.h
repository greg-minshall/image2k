#ifndef __image2k_h__
#define __image2k_h__


/*
 * how to compute luminance
 * from netpbm/ppm.h
 */
#define PPM_LUMINR 0.2989
#define PPM_LUMING 0.5866
#define PPM_LUMINB 0.1145

#define PPM_RGB_TO_LUM(r,g,b)                                        \
    (((r)*PPM_LUMINR)+((g)*PPM_LUMING)+((b)*PPM_LUMINB))


/*
 * callouts from Imlib2 and MagickWand supporting code back to the
 * application code.  the (void *) "cookie" parameter allows the
 * application to pass in a correlation parameter (to recover some
 * data structure between calls in and calls out).
 */

typedef struct {
    /*
     * the (void *) "cookie" parameter allows the application to pass
     * in a correlation parameter (to recover some data structure
     * between calls in and calls out).
     */
    void *cookie;
    /*
     * some calls that might need to be over-ridden (for R, say).
     */
    int (*fprintf)(FILE * restrict stream, const char *restrict format, ...);
    void (*exit)(int status);
    void *(*malloc)(size_t size);
} im2k_t, *im2k_p;

/*
 * when an image file is opened, register the file name, geometry, and
 * height, check for compatibility
 */
typedef void (*fhwcall_t)(im2k_p im2k,
                          const char *file,
                          unsigned int height,
                          unsigned int width,
                          unsigned int depth);
/*
 * for each pixel in the file, do something
 */
typedef void (*process_t)(im2k_p im2k,
                          int i,
                          float red,
                          float green,
                          float blue,
                          float alpha);
/*
 * when creating an output file, return the pixel RGB values at that
 * location
 */
typedef void (*getpixels_t)(im2k_p im2k,
                            int i,
                            float *red,
                            float *green,
                            float *blue,
                            float *alpha);


/*
 * callouts from the application code into the Imlib2 and MagickWand
 * supporting code
 */


/*
 * read a file, and process pixels one at a time
 */
typedef void (*readfile_t)(im2k_p im2k,
                           const char *file,
                           fhwcall_t dofhw,
                           process_t dopix);

/*
 * write a file, accessing the pixels one at a time
 */
typedef void (*writefile_t)(im2k_p im2k,
                            const char *ofile,
                            unsigned int hhh,
                            unsigned int www,
                            unsigned int depth,
                            getpixels_t getpixels);



/*
 * define the actual routines in the Imlib2 and MagickWand supporting
 * code.
 */

#if defined(HAVE_IMLIB2)
/*
 * process a file with imlib2
 */
void readfile2(im2k_p im2k, const char *file,
               fhwcall_t dofhw, process_t dopix);
void writefile2(im2k_p im2k, const char *ofile,
                unsigned int hhh, unsigned int www, unsigned int depth,
                getpixels_t getpixels);
#endif /* defined(HAVE_IMLIB2) */

#if defined(HAVE_MAGICKWAND)
/*
 * process file with imagemagick
 */ 
void readfilek(im2k_p im2k, const char *file,
               fhwcall_t dofhw, process_t dopix);
void writefilek(im2k_p im2k, const char *ofile,
                unsigned int hhh, unsigned int www, unsigned int depth,
                getpixels_t getpixels);
#endif /* defined(HAVE_MAGICKWAND) */

#endif /* ndef __image2k_h__ */
