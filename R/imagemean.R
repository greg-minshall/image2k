chkcompat <- function(pm1, pm2) {
  ## make sure the images have the same channels
  if (!identical(pm1@channels, pm2@channels)) {
    stop("images have different color channels: \"%s\" versus \"%s\"",
         sprintf("%s ", pm1@channels), sprintf("%s ", pm2@channels));
  }
  ## make sure the images have the same size
  if (!identical(pm1@size, pm2@size)) {
    stop(sprintf("images have different sizes: %dx%d versus %dx%d",
                 pm1@size[1], pm1@size[2], pm2@size[1], pm2@size[2]));
  }
}


imagemean <- function(file, ..., output, with.imlib2=TRUE, with.magickwand=TRUE) {
  ##  flatten input.  file may have been specified c(file1, file2,
  ## ...), and this will take care of that
  files <- c(file, ...);

  if ((!missing(output)) && file.exists(output)) {
    stop(sprintf("attempt to overwrite an existing file: %s", output));
  }

  nfiles = 0;
  for (f in files) {
    pm <- read.image2k(f, with.imlib2=with.imlib2, with.magickwand=with.magickwand);
    if (nfiles == 0) {
      sum <- pm;
    } else {
      ## add everything up
      for (chan in sum@channels) {
        slot(sum, chan) = slot(sum, chan) + slot(pm, chan);
      }
    }
    nfiles = nfiles+1;
  }

  for (chan in sum@channels) {
    slot(sum, chan) = slot(sum, chan)/nfiles;
  }

  if (missing(output)) {
    return(sum);
  } else {
    write.image2k(output, pm);
  }
}
