\name{qflipy}
\alias{qflipY}
\title{Flip the Y axis}
\usage{qflipY(ymax, ymin=0)
\method{qflipY}{numeric}(ymax, ymin=0)
\method{qflipY}{QRect}(ymax, ymin=0)
}
\description{Generate transform for flipping Y axis.}
\value{\code{qflipY}: A \code{QTransform} object that will transform points by
flipping the axis.

}
\seealso{\code{\link{qvmap}}}
\author{Michael Lawrence}
\arguments{\item{ymax}{Maximum Y value or a rectangle (\code{QRect} or matrix)}
\item{ymin}{Minimum Y value}
}
\alias{qflipY.numeric}
\alias{qflipY.QRectF}
\alias{qflipY.QRect}

