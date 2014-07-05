#!/usr/bin/env Rscript

## a very simple test:  read in an image file and then write it out, making sure 

args <- commandArgs(TRUE);
arg1 <- 1;

## apparently pixmap (which comes in with image2k) requires methods,
## and quietly=TRUE doesn't seemt to propagate down.
require(methods, quietly=TRUE);
require(digest, quietly=TRUE);

if (args[1] == "--image2klib") {
  require(image2k, lib.loc=args[2], quietly=TRUE);
  arg1 <- arg1+2;
} else {
  require(image2k, quietly=TRUE);
}

for (i in arg1:length(args)) {
  file <- args[i];
  comps <- strsplit(file, .Platform$file.sep)[[1]];
  lcomp <- comps[length(comps)];
  im2k <- read.image2k(file);
  for (chan in sort(im2k@channels)) {
    cat(sprintf("%s, %s, %s\n", lcomp, chan, digest(slot(im2k, chan))))
  }
}
