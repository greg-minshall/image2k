## dyn.load("image2k.dylib");

calc.usemagickwand <- function(with.imlib2, with.magickwand) {
  have_imlib2 = ..External("image2khaveimlib2");
  have_magickwand = ..External("image2khavemagickwand");
  
  if (!(with.imlib2 || with.magickwand)) {
    stop("read.image2k: can't turn off *both* with.imlib2 *and* with.magickwand");
  }

  if (have_imlib2 && have_magickwand) {
    if (with.imlib2 && with.magickwand) {
      usemagickwand = FALSE;            # XXX default, sort of
    } else if (with.imlib2) {
      usemagickwand = FALSE;
    } else if (with.magickwand) {
      usemagickwand = TRUE;
    }
  } else if (have_imlib2) {
    if (with.imlib2) {
      usemagickwand = FALSE;
    } else {
      stop("read.image2k: with_imlib=FALSE: no magickwand, so with.imlib2 can only be true");
    }
  } else if (have_magickwand) {
    if (with.magickwand) {
      usemagickwand = TRUE;
    } else {
      stop("read.image2k: with.magick=FALSE: no magickwand, so with.imlib2 can only be true");
    }
  } else {
    stop("read.image2k: should not occur: neither Imlib2 nor MagickWand available (so, initial install should have failed)");
  }
  usemagickwand;
}

## with the default parameters with.imlib2=TRUE, with.magick=TRUE, it
## is implementation dependent which library will be used for a given
## image.
read.image2k <- function(file, with.imlib2=TRUE, with.magickwand=TRUE) {
  usemagickwand <- calc.usemagickwand(with.imlib2=with.imlib2,
                                      with.magickwand=with.magickwand);

  im2k <- .External("image2kread", file=file, usemagickwand=usemagickwand);
  
  if ((!all(dim(im2k$red) == dim(im2k$green))) ||
      (!all(dim(im2k$green) == dim(im2k$blue)))) {
    stop("R, G, and B matrices must be of the same size");
  }

  ## duplicate some code from pixmap.R (for efficiency)
  datamax <- max(im2k$red, im2k$green, im2k$blue);
  datamin <- min(im2k$red, im2k$green, im2k$blue);

  if(datamax>1 || datamin<0) {
    im2k$red <- (im2k$red - datamin)/(datamax-datamin);
    im2k$green <- (im2k$green - datamin)/(datamax-datamin);
    im2k$blue <- (im2k$blue - datamin)/(datamax-datamin);
  }

  z = new("pixmapRGB", pixmap(im2k$red, ...));
  z@red = im2k$red;
  z@green = im2k$green;
  z@blue = im2k$blue;

  z
}

write.image2k <- function(file, pm, depth=8,
                          with.imlib2=TRUE, with.magickwand=TRUE) {
  usemagickwand <- calc.usemagickwand(with.imlib2=with.imlib2,
                                      with.magickwand=with.magickwand);

  .External("image2kwrite", file=file,
            red=pm@red, green=pm@green, blue=pm@blue,
            depth=depth, usemagickwand=usemagickwand);
}
