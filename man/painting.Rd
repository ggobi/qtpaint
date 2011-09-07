\name{painting}
\alias{qdrawLine}
\title{Drawing API}
\usage{qdrawLine(p, x, y, stroke)
qdrawSegment(p, x0, y0, x1, y1, stroke)
qdrawPoint(p, x, y, stroke)
qdrawRect(p, xleft, ybottom, xright, ytop, stroke, fill)
qdrawCircle(p, x, y, r, stroke, fill)
qdrawPolygon(p, x, y, stroke, fill)
qdrawPath(p, path, stroke, fill)
qdrawText(p, text, x, y, halign=c("center", "left", "right"), valign=c("center",
    "basecenter", "baseline", "bottom", "top"), rot=0, color, cex=1,
    hcex=cex, vcex=cex)
qdrawImage(p, image, x, y)
qdrawGlyph(p, path, x, y, cex, stroke, fill)
}
\description{These functions constitute the primary drawing API. There is
support for drawing points, polylines, segments, circles, rectangles,
polygons, vector paths, text, images and plot glyphs.}
\author{Michael Lawrence}
\arguments{\item{p}{The paint context}
\item{x}{The X coordinate vector, recycled. For polygons and
polylines, NA values separate the graphical primitives.}
\item{y}{The Y coordinate vector, recycled. For polygons and
polylines, NA values separate the graphical primitives.}
\item{stroke}{The vector of stroke colors, either a C++
\code{QColor} object, a matrix returned by \code{\link{col2rgb}}
or any valid input to \code{col2rgb}, recycled}
\item{x0}{The vector of first X coordinates, recycled}
\item{y0}{The vector of first Y coordinates, recycled}
\item{x1}{The vector of second X coordinates, recycled}
\item{y1}{The vector of second Y coordinates, recycled}
\item{fill}{The vector of fill colors, either a C++
\code{QColor} object, a matrix returned by \code{\link{col2rgb}}
or any valid input to \code{col2rgb}, recycled}
\item{xleft}{The vector of left X coordinates for a rectangle, recycled}
\item{ybottom}{The vector of bottom Y coordinates for a rectangle, recycled}
\item{xright}{The vector of right X coordinates for a rectangle, recycled}
\item{ytop}{The vector of top Y coordinates for a rectangle, recycled}
\item{r}{The radius of the circle, in device coordinates, recycled}
\item{path}{A C++ \code{QPainterPath} object describing the glyph,
or a list of such objects for \code{qdrawPath}.}
\item{text}{The vector of strings to draw, recycled}
\item{halign}{The side of the text to horizontally align to the coordinate}
\item{valign}{The side of the text to vertically align to the
coordinate. Besides the obvious alignment options, there are two
different ways to center the text: according to the entire text
extents ("center") or only according to the region above the
baseline ("basecenter"). The former may be better for plotting
text, while the latter may be better for labeling.}
\item{rot}{The vector of the text rotations, in degrees, recycled}
\item{color}{The stroke color of the text}
\item{cex}{The vector of numeric expansion factors of the glyphs, recycled}
\item{hcex}{The vector of numeric horizontal expansion factors of
the glyphs, recycled. Overrides the \code{cex} in the horizontal
direction.}
\item{vcex}{The vector of numeric vertical expansion factors of
the glyphs, recycled. Overrides the \code{cex} in the vertical
direction.}
\item{image}{A C++ \code{QImage} object}
}
\alias{qdrawSegment}
\alias{qdrawPoint}
\alias{qdrawRect}
\alias{qdrawCircle}
\alias{qdrawPolygon}
\alias{qdrawPath}
\alias{qdrawText}
\alias{qdrawImage}
\alias{qdrawGlyph}

