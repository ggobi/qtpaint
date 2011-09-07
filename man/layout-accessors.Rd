\name{layout-accessors}
\alias{[<-.Qanviz::Layer}
\title{Grid layout accessors}
\usage{
\method{[}{`Qanviz::Layer`}(x, i=0, j=0, rowSpan=1, colSpan=1) <- value
\method{[}{`Qanviz::Layer`}(x, i=0, j=0)
}
\description{Add or retrieve a layer to or from the grid layout of another layer}
\author{Michael Lawrence}
\arguments{\item{x}{Parent layer}
\item{i}{0-based row position}
\item{j}{0-based column position}
\item{rowSpan}{Number of rows spanned by \code{value}}
\item{colSpan}{Number of columns spanned by \code{value}}
\item{value}{The layer to add to the layout}
}
\alias{[.Qanviz::Layer}

