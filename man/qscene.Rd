\name{qscene}
\alias{qscene}
\title{Create scenes}
\usage{qscene(parent)
}
\description{Create a scene, a container of layers}
\details{Often, a \code{PlotView} object is passed as \code{parent}, so
that the scene is not deleted until the view is destroyed.}
\value{A C++ \code{QGraphicsScene} object}
\author{Michael Lawrence}
\arguments{\item{parent}{If non-NULL, a C++ \code{QObject} object that owns
the memory of the scene}
}

