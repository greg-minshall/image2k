\name{image2k-package}

\alias{image2k}
\alias{read.image2k}
\alias{write.image2k}

\docType{package}

\title{create pixmap images from various image file formats}

\description{
  Image2k allows pixmap images to be read from, or written to, files of
  various image file formats, using Imlib2 and/or ImageMagick\'s
  WandMagick interface.
}
\usage{
read.image2k(file, with.imlib2=TRUE, with.magickwand=TRUE)
write.image2k(file, pm, depth=8, with.imlib2=TRUE, with.magickwand=TRUE)
}
\arguments{
  \item{file}{pathname of file to read or write}
  \item{with.imlib2}{whether to possibly choose Imbli2 for reading/writing}
  \item{with.magickwand}{whether to possibly choose MagickWand for reading/writing}
  \item{pm}{pixmap containing image to be written}
  \item{depth}{number of bits per pixel channel; normally 8, sometimes 16.}
}

\details{
\tabular{ll}{
Package: \tab image2k\cr
Type: \tab Package\cr
Version: \tab 1.0\cr
Date: \tab 2014-06-24\cr
License: \tab MIT\cr
}

The
\href{http://cran.r-project.org/web/packages/pixmap/index.html}{pixmap}
package is able to read and write
\href{https://en.wikipedia.org/wiki/Netpbm_format}{NetPBM} files.  An
add-on package
\href{http://cran.r-project.org/web/packages/rtiff/index.html}{rtiff}
allows a pixmap image to be created from
\href{https://en.wikipedia.org/wiki/Tagged_Image_File_Format}{TIFF}
files (through the use of the
\href{http://www.remotesensing.org/libtiff/}{libtiff} library).  Both
pixmap and rtiff have other functions.

Image2k is another add-on to pixmap, and adds the ability to read or
write image files in any format that is understood by either the
\href{http://docs.enlightenment.org/api/imlib2/html/}{Imlib2} libraries
or Imagemagick\'s
\href{http://www.imagemagick.org/script/magick-wand.php}{MagickWand}
libraries.  (Of course, one or both of these libraries needs to be
installed on your machine at the time image2k is installed, and is run,
for this to be possible.)

\code{read.image2k} reads an image file, and returns a pixmapRGB with
that files contents in it.

\code{write.image2kwrite} attempts to write the image contained in pm to
the image file.  the depth of the image (default \code{8}) can be
specified.

note that image file formats are determined (by whichever of the Imlib2
or MagickWand libraries is called) based on the filename extension.  so,
a file of foo.jpg would be assumed to be a JPEG file.

the with.imlib2 and with.magickwand parameters each default to TRUE.  in
this case, assuming the system has both libraries installed, image2k
makes an arbitrary decision of which library to use, and sticks with
that decision.  (I.e., it is *NOT* the case that image2k first tries one
library and, should that one fail, switches over to the other library.)

}
\value{
  \code{read.image2k}, if succesful, returns a pixmapRGB object.
}
\author{
Greg Minshall <\email{minshall@acm.org}>
}
\keyword{ package }
\seealso{
~~ Optional links to other man pages, e.g. ~~
\code{\link{pixmap}}
\code{\link{rtiff}}
}
\examples{
## a simple program to read in a variable number of files (of the
## same dimension) and produce an output where each pixel is the
## average of the values in the corresponding locations in the
## input files.

\dontrun{
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
}
}