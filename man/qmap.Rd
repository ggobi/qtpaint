\name{qvmap}
\alias{qvmap}
\title{Mapping coordinates}
\usage{qvmap(m, x, y)
}
\description{Transforms X and Y coordinates with a Qt-style transformation
matrix. The advantage over direct use of Qt is vectorization.}
\value{The mapped coordinates, as a two column (X, Y) matrix,
unless \code{y} is missing, in which case an attempt is made to
coerce the result to the class of \code{x}, if any.}
\author{Michael Lawrence}
\arguments{\item{m}{A matrix encoding the transformation, or something
coercible to a matrix, like a C++ \code{QTransform} instance}
\item{x}{X coordinates; if \code{y} is missing, should be
something coercible to a numeric vector or matrix. If the vector
coercion succeeds, the vector is coerced to a matrix with
\code{matrix(x, ncol = 2, byrow = TRUE)}. The first column is taken
as X, the second as Y.}
\item{y}{Y coordinates, optional}
}

