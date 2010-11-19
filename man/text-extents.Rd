\name{text-extents}
\alias{qtextExtents}
\title{Text extents}
\usage{qtextExtents(p, text)
qstrWidth(p, text)
qstrHeight(p, text)
qfontMetrics(p)
}
\description{Get text extents and font metrics}
\value{\code{qtextExtents}: A matrix representing the text bounds for
\code{qtextExtents}), a number for \code{qstrWidth} and
\code{qstrHeight}, or a vector with the ascent and descent for
\code{qfontMetrics}

}
\author{Michael Lawrence}
\arguments{\item{p}{The paint context}
\item{text}{The text to analyze}
}
\alias{qstrWidth}
\alias{qstrHeight}
\alias{qfontMetrics}

