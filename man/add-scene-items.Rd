\name{add-scene-items}
\alias{qscene.points}
\title{Adding scene items}
\usage{qscene.points(s, x, y, radius=1)
qscene.lines(s, x, y, lwd=0)
qscene.segments(s, x1, y1, x2, y2, lwd=0)
qscene.rect(s, x, y, w=1, h=1)
}
\description{These functions constitute a vectorized API for adding graphical
primitive items into a scene. Each primitive is represented by a
separate canvas item. This is a different paradigm from the
layer-based painting, which has a more appropriate design for most
interactive graphics. Most of the time, the user should ignore this API.}
\author{Deepayan Sarkar}
\arguments{\item{s}{The scene, a C++ \code{QGraphicsScene}}
\item{x}{The X coordinates, as expected by \code{\link{xy.coords}}, recycled}
\item{y}{The Y coordinates, as expected by \code{\link{xy.coords}}, recycled}
\item{radius}{The scalar radius for the points, in device coordinates}
\item{lwd}{The vector of line widths, in device coordinates, recycled}
\item{x1}{The first X coordinates, recycled}
\item{y1}{The first Y coordinates, recycled}
\item{x2}{The second X coordinates, recycled}
\item{y2}{The second Y coordinates, recycled}
\item{w}{Vector of rectangle widths, recycled}
\item{h}{Vector of rectangle heights, recycled}
}
\alias{qscene.lines}
\alias{qscene.segments}
\alias{qscene.rect}

