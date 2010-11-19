\name{qupdate}
\alias{qupdate}
\title{Updating drawings}
\usage{qupdate(x)
\method{qupdate}{QGraphicsView}(x)
\method{qupdate}{QGraphicsScene}(x)
\method{qupdate}{QGraphicsItem}(x)
}
\description{Force a redraw of a layer or scene, clearing the cache. This needs
to be called whenever the drawing would change, e.g., if the data
or some visual attribute has changed. There is no automatic way
for qtpaint to detect this.}
\author{Michael Lawrence}
\arguments{\item{x}{The object, usually a layer or scene, to be redrawn}
}
\alias{qupdate.QGraphicsView}
\alias{qupdate.QGraphicsScene}
\alias{qupdate.QGraphicsItem}

