\name{qglyphs}
\alias{qglyphCircle}
\title{Glyph constructors}
\usage{qglyphCircle(r=5)
qglyphSquare(x=5)
qglyphTriangle(x=5, direction=c("up", "down"))
qglyphText(text="X", size=12)
qglyphSegment(x=5, direction=0)
}
\description{Some glyph constructors for simple glyphs like circle, square and
triangle. For use with \code{\link{qdrawGlyph}}.}
\value{\code{qglyphCircle}: C++ \code{QPainterPath} instance for use with
\code{\link{qdrawGlyph}}

}
\author{Michael Lawrence}
\arguments{\item{r}{Radius of the circle}
\item{x}{Length of one side of the square or triangle or the segment}
\item{direction}{Whether the triangle is pointing up or down, or the direction of the segment (unit: radian)}
\item{text}{The text of the text glyph}
\item{size}{The font size of the text glyph}
}
\alias{qglyphSquare}
\alias{qglyphTriangle}
\alias{qglyphText}
\alias{qglyphSegment}
