## with the default parameters with.imlib2=TRUE, with.magick=TRUE, it
## is implementation dependent which library will be used for a given
## image.
read.image2k <- function(file, with.imlib2=TRUE, with.magickwand=TRUE ...) {
  head <- read.image.head(con);       # XXX errors?
  retval <- read.image.data(con, head);
  retval;
}

write.image2k <- function(file, pmap, with.imlib2=TRUE, with.magickwand=TRUE) {
}
