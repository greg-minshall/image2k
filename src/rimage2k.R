read.image <- function(file, ...) {
  head <- read.image.head(con);       # XXX errors?
  retval <- read.image.data(con, head);
  retval;
}

read.image.head(file) {
  
}

read.image.data(file, imhead) {
  
}
