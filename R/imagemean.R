setMethod("+", signature(e1="pixmap", e2="pixmap"),
function(e1, e2) {
  ## make sure the images have the same channels
  if (!all(e1@channels == e2@channels)) {
    stop("images have different color channels: \"%s\" versus \"%s\"",
         sprintf("%s ", e1@channels), sprintf("%s ", e2@channels));
  }
  ## make sure the images have the same size
  if (!all(e1@size == e2@size)) {
    stop(sprintf("images have different sizes: %dx%d versus %dx%d",
                 e1@size[1], e1@size[2], e2@size[1], e2@size[2]));
  }

  for (chan in e1@channels) {
    slot(e1, chan) = slot(e1, chan) + slot(e2, chan);
  }

  e1
})


imagemean <- function(file, ..., with.imlib2=TRUE, with.magickwand=TRUE) {
  ##  flatten input.  file may have been specified c(file1, file2,
  ## ...), and this will take care of that
  files <- c(file, ...);

  nfiles = 0;
  for (f in files) {
    pm <- read.image2k(f, with.imlib2=with.imlib2, with.magickwand=with.magickwand);
    if (nfiles == 0) {
      sum <- pm;
    } else {
      sum <- sum + pm;
    }
    nfiles = nfiles+1;
  }

  for (chan in sum@channels) {
    slot(sum, chan) = slot(sum, chan)/nfiles;
  }

  sum;
}
