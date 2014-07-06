#!/usr/bin/env Rscript

# read in an image file, output the md5 hash of its channels

## apparently pixmap (which comes in with image2k) requires methods,
## and quietly=TRUE doesn't seemt to propagate down.
require(methods, quietly=TRUE);
require(digest, quietly=TRUE);
require(getopt, quietly=TRUE);
## image2k is required below (in case it comes from a special library)

usage <- function(spec) {
  cat(getopt(spec, usage=TRUE));
  q(status=1);
}


md5.image2k <- function(file, ...) {
  comps <- strsplit(file, .Platform$file.sep)[[1]];
  lcomp <- comps[length(comps)];
  im2k <- read.image2k(file, ...);
  res <- matrix(nrow=0, ncol=3);
  for (chan in sort(im2k@channels)) {
    res <- rbind(res, c(file=lcomp,
                        chan=chan,
                        md5=digest(slot(im2k, chan),algo="md5")));
  }
  print(res);
  res;
}

parserun <- function(args) {
  spec <- matrix(c(
    "file",       "f", 1, "character", "input image filename",
    "help",       "h", 0, "character", "print this usage information",
    "image2klib", "i", 1, "character", "library location of image2k library",
    "pref.lib",   "p", 1, "character", "string with one or two unique characters from  {2, k} (which library to use)"
    ), byrow=TRUE, ncol=5);


  opts <- getopt(spec=spec, opt=args);

  if (!is.null(opts$help)) {
    usage(spec);
  }

  if (is.null(opts$file)) {
    cat("--file|-f not specified\n");
    usage(spec);
  }

  if (!is.null(opts$image2klib)) {
    require(image2k, lib.loc=opts$image2klib, quietly=TRUE);
  } else {
    require(image2k, lib.loc=opts$image2klib, quietly=TRUE);
  }

  file <- opts$file;

  res <- md5.image2k(file=file, pref.lib=opts$pref.lib);

  print(res);
  print(nrow(res));
  for (i in 1:nrow(res)) {
    cat(sprintf("%s %s %s\n", res[i,"file"], res[i,"chan"], res[i,"md5"]));
  }
}

## if we are called from Rscript, parse arguments and run the command
## (but, if sourced, don't do anything useful).
if (!is.na(get_Rscript_filename())) {
  parserun(commandArgs(TRUE));
}
