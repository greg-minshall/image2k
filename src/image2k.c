/*
 * this file contains routines to read and write image files, using
 * either the Imlib2
 * http://docs.enlightenment.org/api/imlib2/html/index.html or
 * MagickWand, the C API component of ImageMagick
 * http://www.imagemagick.org/script/magick-wand.php
 *
 * the read routines load an image, then call out to register the file
 * with its attributes (rows, columns, and depth).  they then access
 * each pixel in the image file, calling out to give the caller a
 * chance to look at each pixel.  after that, the image file is
 * closed, and the original call returns.
 *
 * the write routines are called with the name of the output file, and
 * the output file's dimensions.  a call out is made to access each
 * pixel, and then the output file is created, and the original call
 * returns.
 */

#include <stdio.h>
#include <stdlib.h>

#include "config.h"

#if defined(HAVE_IMLIB2)
#include <Imlib2.h>
#endif /* defined(HAVE_IMLIB2) */

#if  defined(HAVE_MAGICKWAND)
#include <wand/MagickWand.h>
#endif /* defined(HAVE_MAGICKWAND) */

/*
 * this file contains Imlib2 and MagickWand (from ImageMagick)
 * routines to support the utilities.
 */

#include "image2k.h"

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

#define IMLIB2_DEPTH 8          /* 8 bits per channel */

#define GetA(argb) (((argb)>>24)&0xff)
#define GetR(argb) (((argb)>>16)&0xff)
#define GetG(argb) (((argb)>> 8)&0xff)
#define GetB(argb) (((argb)    )&0xff)

#define PutA(argb, a) (((argb)&0x00ffffff) | (((a)<<24)&0xff000000))
#define PutR(argb, b) (((argb)&0xff00ffff) | (((b)<<16)&0x00ff0000))
#define PutG(argb, g) (((argb)&0xffff00ff) | (((g)<< 8)&0x0000ff00))
#define PutB(argb, r) (((argb)&0xffffff00) | (((r)    )&0x000000ff))


/* http://docs.enlightenment.org/api/imlib2/html/ */

/*
 * return a string "explainig" the reason a load_image (or save_image)
 * failed.
 */

static char *
image_decode_load_error(Imlib_Load_Error error) {
    switch (error) {
    case IMLIB_LOAD_ERROR_NONE:
        return "imlib_load_error_none";
    case IMLIB_LOAD_ERROR_FILE_DOES_NOT_EXIST:
        return "imlib_load_error_file_does_not_exist";
    case IMLIB_LOAD_ERROR_FILE_IS_DIRECTORY:
        return "imlib_load_error_file_is_directory";
    case IMLIB_LOAD_ERROR_PERMISSION_DENIED_TO_READ:
        return "imlib_load_error_permission_denied_to_read";
    case IMLIB_LOAD_ERROR_NO_LOADER_FOR_FILE_FORMAT:
        return "imlib_load_error_no_loader_for_file_format";
    case IMLIB_LOAD_ERROR_PATH_TOO_LONG:
        return "imlib_load_error_path_too_long";
    case IMLIB_LOAD_ERROR_PATH_COMPONENT_NON_EXISTANT:
        return "imlib_load_error_path_component_non_existant";
    case IMLIB_LOAD_ERROR_PATH_COMPONENT_NOT_DIRECTORY:
        return "imlib_load_error_path_component_not_directory";
    case IMLIB_LOAD_ERROR_PATH_POINTS_OUTSIDE_ADDRESS_SPACE:
        return "imlib_load_error_path_points_outside_address_space";
    case IMLIB_LOAD_ERROR_TOO_MANY_SYMBOLIC_LINKS:
        return "imlib_load_error_too_many_symbolic_linksn";
    case IMLIB_LOAD_ERROR_OUT_OF_MEMORY:
        return "imlib_load_error_out_of_memory";
    case IMLIB_LOAD_ERROR_OUT_OF_FILE_DESCRIPTORS:
        return "imlib_load_error_out_of_file_descriptors";
    case IMLIB_LOAD_ERROR_PERMISSION_DENIED_TO_WRITE:
        return "imlib_load_error_permission_denied_to_write";
    case IMLIB_LOAD_ERROR_OUT_OF_DISK_SPACE:
        return "imlib_load_error_out_of_disk_space";
    case IMLIB_LOAD_ERROR_UNKNOWN:
        return "imlib_load_error_unknown";
    default:
        return "unknown/invalid error";
    }
}

/*
 * process a file with imlib2
 */
void
readfile2(void *cookie, const char *file, fhwcall_t dofhw, process_t dopix) {
    Imlib_Image x;              /* imlib2 context */
    DATA32 *data;               /* actual image data */
    int i, val, w, h, len;

    x  = imlib_load_image_without_cache(file);
    if (x == NULL) {
        Imlib_Load_Error error_return;
        x = imlib_load_image_with_error_return(file, &error_return);
        fprintf(stderr, "unable to open \"%s\": %s\n",
                file, image_decode_load_error(error_return));
        exit(7);
        /*NOTREACHED*/
    }
    imlib_context_set_image(x);

    w = imlib_image_get_width();
    h = imlib_image_get_height();
    len = w*h;

    (dofhw)(cookie, file, h, w, IMLIB2_DEPTH);

    data = imlib_image_get_data_for_reading_only();

    for (i = 0; i < len; i++) {
        val = data[i];
        (dopix)(cookie, i,
                GetR(val)/255.0, GetG(val)/255.0, GetB(val)/255.0, GetA(val)/255.0);
    }
    imlib_free_image_and_decache();
}


/*
 * finish processing with imlib2
 */
void
writefile2(void *cookie, const char *ofile, unsigned int hhh,
           unsigned int www, unsigned int depth, getpixels_t getpixels) {
    int i, r, g, b, a, val, len;
    Imlib_Image outimage;
    DATA32 *outdata;
    Imlib_Load_Error imerr;

    if (depth != 8) {
        fprintf(stderr,
                "%s:%d: Imlib2 is unable to write an image of %d bits per channel (only 8 is allowed).\n",
                __FILE__, __LINE__, depth);
        exit(8);
        /*NOTREACHED*/
    }

    len = hhh*www;
    outdata = (DATA32 *)malloc(len*(sizeof (DATA32)));
    if (outdata == NULL) {
        fprintf(stderr, "no room for output buffer\n");
        exit(9);
        /*NOTREACHED*/
    }

    for (i = 0; i < len; i++) {
        val = 0;
        float fr, fg, fb, fa;

        (getpixels)(cookie, i, &fr, &fg, &fb, &fa);
        r = fr*255.0;
        g = fg*255.0;
        b = fb*255.0;
        a = fa*255.0;
        val |= PutR(val, r);
        val |= PutG(val, g);
        val |= PutB(val, b);
        val |= PutA(val, a);
        outdata[i] = val;
    }
    outimage = imlib_create_image_using_data(www, hhh, outdata);
    if (outimage == NULL) {
        fprintf(stderr, "unable to create output image structures (internal): ");
        perror("");
        exit(10);
        /*NOTREACHED*/
    }
    imlib_context_set_image(outimage);    
    imlib_save_image_with_error_return(ofile, &imerr);
    if (imerr != IMLIB_LOAD_ERROR_NONE) {
        fprintf(stderr, "error saving output file \"%s\": ", ofile);
        perror("");
        exit(10);
        /*NOTREACHED*/
    }
    imlib_free_image_and_decache();
}
#endif /* defined(HAVE_IMLIB2) */

#if defined(HAVE_MAGICKWAND)

/*
 * the ImageMagick bits are mostly copied from the sample program
 * here:
 * http://www.imagemagick.org/script/magick-wand.php
 */

static void
ThrowWandException(MagickWand *wand) {
    char *description;

    ExceptionType severity;

    description=MagickGetException(wand,&severity);
    (void) fprintf(stderr,"%s %s %lu %s\n",GetMagickModule(),description);
    description=(char *) MagickRelinquishMemory(description);
    exit(-1);
    /*NOTREACHED*/
}


void
readfilek(void *cookie, const char *file, fhwcall_t dofhw, process_t dopix) {
    long y;
    MagickBooleanType status;
    MagickPixelPacket pixel;
    MagickWand *image_wand;
    PixelIterator *iterator;
    PixelWand **pixels;
    register long x;
    unsigned long width;
    int i = 0, h, w, d;

    /* Read an image. */
    MagickWandGenesis();
    image_wand = NewMagickWand();
    status = MagickReadImage(image_wand, file);
    if (status == MagickFalse) {
        ThrowWandException(image_wand);
        /*NOTREACHED*/
    }
    h = MagickGetImageHeight(image_wand);
    w = MagickGetImageWidth(image_wand);
    d = MagickGetImageDepth(image_wand);

    (dofhw)(cookie, file, h, w, d);

    iterator = NewPixelIterator(image_wand);
    if (iterator == (PixelIterator *) NULL) {
        ThrowWandException(image_wand);
        /*NOTREACHED*/
    }
    for (y=0; y < h; y++) {
        pixels = PixelGetNextIteratorRow(iterator,&width);
        if (pixels == (PixelWand **) NULL) {
            break;
        }
        for (x=0; x < (long) width; x++) {
            // PixelGet* returns in range [0,1); we like [0..255]
            (dopix)(cookie, i,
                    PixelGetRed(pixels[x]),
                    PixelGetGreen(pixels[x]),
                    PixelGetBlue(pixels[x]),
                    PixelGetAlpha(pixels[x]));
            i++;
        }
    }
    if (y < (long) MagickGetImageHeight(image_wand)) {
        ThrowWandException(image_wand);
        /*NOTREACHED*/
    }
    iterator = DestroyPixelIterator(iterator);
    image_wand = DestroyMagickWand(image_wand);
}

// this bit is based on:
// http://members.shaw.ca/el.supremo/MagickWand/grayscale.htm

void
writefilek(void *cookie, const char *ofile,
           unsigned int hhh, unsigned int www,
           unsigned int depth, getpixels_t getpixels) {
    MagickWand *m_wand = NULL;
    PixelWand *p_wand = NULL;
    PixelIterator *iterator = NULL;
    PixelWand **pixels = NULL;
    unsigned long x, y;
    int i;

    MagickWandGenesis();
    m_wand = NewMagickWand();
    if (!MagickSetDepth(m_wand, depth)) {
        ThrowWandException(m_wand);
        /*NOTREACHED*/
    }
    p_wand = NewPixelWand();
    PixelSetColor(p_wand, "white");
    if (!MagickNewImage(m_wand, www, hhh, p_wand)) {
        ThrowWandException(m_wand);
        /*NOTREACHED*/
    }
    // Get a new pixel iterator 
    iterator = NewPixelIterator(m_wand);
    i = 0;
    for(y = 0; y < hhh; y++) {
        // Get the next row of the image as an array of PixelWands
        pixels = PixelGetNextIteratorRow(iterator, &x);
        // Set the row of wands to a simple gray scale gradient
        for(x = 0; x < www; x++) {
            float red, green, blue, alpha;
            (getpixels)(cookie, i, &red, &green, &blue, &alpha);
            PixelSetRed(pixels[x], red);
            PixelSetGreen(pixels[x], green);
            PixelSetBlue(pixels[x], blue);
            PixelSetAlpha(pixels[x], alpha);
            i++;
        }
        // Sync writes the pixels back to the m_wand
        PixelSyncIterator(iterator);
    }
    if (!MagickWriteImage(m_wand, ofile)) {
        ThrowWandException(m_wand);
        /*NOTREACHED*/
    }
    // Clean up
    iterator = DestroyPixelIterator(iterator);
    DestroyMagickWand(m_wand);
    DestroyPixelWand(p_wand);
    MagickWandTerminus();
}
#endif /* defined(HAVE_MAGICKWAND) */
