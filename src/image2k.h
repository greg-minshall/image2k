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

/*
 * when an image file is opened, register the file name, geometry, and
 * height, check for compatibility
 */
typedef void (*fhwcall_t)(void *cookie,
                          const char *file,
                          unsigned int height,
                          unsigned int width,
                          unsigned int depth);
/*
 * for each pixel in the file, do something
 */
typedef void (*process_t)(void *cookie,
                          int i,
                          float red,
                          float green,
                          float blue,
                          float alpha);
/*
 * when creating an output file, return the pixel RGB values at that
 * location
 */
typedef void (*getpixels_t)(void *cookie,
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
typedef void (*readfile_t)(void *cookie,
                           const char *file,
                           fhwcall_t dofhw,
                           process_t dopix);

/*
 * write a file, accessing the pixels one at a time
 */
typedef void (*writefile_t)(void *cookie,
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
void readfile2(void *cookie, const char *file,
               fhwcall_t dofhw, process_t dopix);
void writefile2(void *cookie, const char *ofile,
                unsigned int hhh, unsigned int www, unsigned int depth,
                getpixels_t getpixels);
#endif /* defined(HAVE_IMLIB2) */

#if defined(HAVE_MAGICKWAND)
/*
 * process file with imagemagick
 */ 
void readfilek(void *cookie, const char *file,
               fhwcall_t dofhw, process_t dopix);
void writefilek(void *cookie, const char *ofile,
                unsigned int hhh, unsigned int www, unsigned int depth,
                getpixels_t getpixels);
#endif /* defined(HAVE_MAGICKWAND) */

#endif /* ndef __image2k_h__ */
