#!/usr/bin/env Rscript

v <- 1:10
nr <- 5976
nc <- 3992

when <- Sys.time();

for (i in v) {
  j <- runif(nr*nc);
  m <- matrix(j, nrow=nr, byrow=FALSE);
}

print(sprintf("byrow=FALSE: %s", Sys.time()-when));

when <- Sys.time();

for (i in v) {
  j <- runif(nr*nc);
  m <- matrix(j, nrow=nr, byrow=TRUE);
}

print(sprintf("byrow=TRUE: %s", Sys.time()-when));

when <- Sys.time();

for (i in v) {
  j <- runif(nr*nc);
  dim(j) <- c(nr, nc);
}

print(sprintf("dim(j) <- c(nr, nc): %s", Sys.time()-when));
