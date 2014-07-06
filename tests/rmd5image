#!/usr/bin/env Rscript

# read in an image file, output the md5 hash of its channels

## apparently pixmap (which comes in with image2k) requires methods,
## and quietly=TRUE doesn't seemt to propagate down.
require(methods, quietly=TRUE);
require(digest, quietly=TRUE);
require(getopt, quietly=TRUE);
## image2k is required below (in case it comes from a special library)

usage <- function() {
  cat(getopt(spec, usage=TRUE));
  q(status=1);
}


spec <- matrix(c(
  "file",       "f", 1, "character", "input image filename",
  "help",       "h", 0, "character", "print this usage information",
  "image2klib", "i", 1, "character", "library location of image2k library",
  "pref.lib",   "p", 1, "character", "string with one or two of \"2\", \"k\" (which library to use)"
  ), byrow=TRUE, ncol=5);


opts <- getopt(spec);

if (!is.null(opts$help)) {
  usage();
}

if (is.null(opts$file)) {
  cat("--file|-f not specified\n");
  usage();
}

if ((!is.null(opts$withk)) && (!is.null(opts$with2))) {
  cat("only one of --withk|-k and --with2|-2 can be specified\n");
  usage();
}

if (!is.null(opts$image2klib)) {
  require(image2k, lib.loc=opts$image2klib, quietly=TRUE);
} else {
  require(image2k, lib.loc=opts$image2klib, quietly=TRUE);
}

file <- opts$file;
comps <- strsplit(file, .Platform$file.sep)[[1]];
lcomp <- comps[length(comps)];
im2k <- read.image2k(file, pref.lib=opts$pref.lib);
for (chan in sort(im2k@channels)) {
  cat(sprintf("%s, %s, %s\n", lcomp, chan, digest(slot(im2k, chan),algo="md5")));
}
