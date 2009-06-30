## R wrappers around the QViz::Painter API
## These are the lowest level wrappers

## TODO: maybe move to a model where 'p' is obtained from a hidden
## variable in the parent (callback) frame.


qmatrix.Painter <- function(x, inverted = FALSE) 
  .Call("qt_qmatrix_Painter", x, as.logical(inverted))

`qmatrix<-.Painter` <- function(x, value) {
  .Call("qt_qsetMatrix_Painter", x, value, PACKAGE = "qtpaint")
}

`qmatrixEnabled<-` <- function(p, value) {
  stopifnot(inherits(p, "Painter"))
  invisible(.Call("qt_qsetMatrixEnabled_Painter", p, as.logical(value)))
}

`qhasStroke<-` <- function(p, value) {
  stopifnot(inherits(p, "Painter"))
  invisible(.Call("qt_qsetHasStroke_Painter", p, as.logical(value)))
}

`qhasFill<-` <- function(p, value) {
  stopifnot(inherits(p, "Painter"))
  invisible(.Call("qt_qsetHasFill_Painter", p, as.logical(value)))
}

.normArgColor <- function(color, len) {
  if (is.null(color))
    return(NULL)
  if (!is.matrix(color) || !is.integer(color) || nrow(color) != 4)
    color <- col2rgb(color, TRUE)
  if (!missing(len)) # might drop to vector here, much faster, C code is OK
    color <- recycleVector(color, 4L*len) 
  color
}

`qstrokeColor<-` <- function(p, value) {
  stopifnot(inherits(p, "Painter"))
  color <- .normArgColor(value)
  invisible(.Call("qt_qsetStrokeColor_Painter", p, color))
}

`qfillColor<-` <- function(p, value) {
  stopifnot(inherits(p, "Painter"))
  color <- .normArgColor(value)
  invisible(.Call("qt_qsetFillColor_Painter", p, color))
}

## 'font' as returned by 'qfont'
`qfont<-` <- function(p, value)
{
  stopifnot(inherits(p, "Painter"))
  invisible(.Call("qt_qsetFont_Painter", p, as.character(value$family),
                  as.integer(value$pointsize), as.integer(value$weight),
                  as.logical(value$italic)))
}

`qlineWidth<-` <- function(p, value) {
  stopifnot(inherits(p, "Painter"))
  invisible(.Call("qt_qsetLineWidth_Painter", p, as.integer(value)))
}

`qdash<-` <- function(p, value) {
  stopifnot(inherits(p, "Painter"))
  invisible(.Call("qt_qsetDashes_Painter", p, as.numeric(value)))
}

`qantialias<-` <- function(p, value) {
  stopifnot(inherits(p, "Painter"))
  invisible(.Call("qt_qsetAntialias_Painter", p, as.logical(value)))
}

qdrawLine <- function(p, x, y, stroke = NULL) {
  stopifnot(inherits(p, "Painter"))
  m <- max(length(x), length(y))
  x <- recycleVector(x, m)
  y <- recycleVector(y, m)
  invisible(.Call("qt_qdrawPolyline_Painter", p, as.numeric(x), as.numeric(y),
                  .normArgColor(stroke, m)))
}

qdrawSegment <- function(p, x0, y0, x1, y1, stroke = NULL) {
  stopifnot(inherits(p, "Painter"))
  m <- max(length(x0), length(y0), length(x1), length(y1))
  x0 <- recycleVector(x0, m)
  y0 <- recycleVector(y0, m)
  x1 <- recycleVector(x1, m)
  y1 <- recycleVector(y1, m)
  invisible(.Call("qt_qdrawSegments_Painter", p, as.numeric(x0), as.numeric(y0),
                  as.numeric(x1), as.numeric(y1),
                  .normArgColor(stroke, m)))
}

qdrawPoint <- function(p, x, y, stroke = NULL) {
  stopifnot(inherits(p, "Painter"))
  m <- max(length(x), length(y))
  x <- recycleVector(x, m)
  y <- recycleVector(y, m)
  invisible(.Call("qt_qdrawPoints_Painter", p, as.numeric(x), as.numeric(y),
                  .normArgColor(stroke, m)))
}

qdrawRect <- function(p, xleft, ybottom, xright, ytop, stroke = NULL,
                      fill = NULL)
{
  stopifnot(inherits(p, "Painter"))
  m <- max(length(xleft), length(ybottom), length(xright), length(ytop))
  xleft <- recycleVector(xleft, m)
  ybottom <- recycleVector(ybottom, m)
  xright <- recycleVector(xright, m)
  ytop <- recycleVector(ytop, m)
  invisible(.Call("qt_qdrawRectangles_Painter", p, as.numeric(xleft),
                  as.numeric(ybottom), as.numeric(xright - xleft),
                  as.numeric(ytop - ybottom), .normArgColor(stroke, m),
                  .normArgColor(fill, m)))
}

qdrawCircle <- function(p, x, y, r, stroke = NULL, fill = NULL) {
  stopifnot(inherits(p, "Painter"))
  m <- max(length(x), length(y), length(r))
  x <- recycleVector(x, m)
  y <- recycleVector(y, m)
  r <- recycleVector(r, m)
  invisible(.Call("qt_qdrawCircle_Painter", p, as.numeric(x), as.numeric(y),
                  as.integer(r), .normArgColor(stroke, m),
                  .normArgColor(fill, m)))
}

qdrawPolygon <- function(p, x, y, stroke = NULL, fill = NULL) {
  stopifnot(inherits(p, "Painter"))
  m <- max(length(x), length(y))
  x <- recycleVector(x, m)
  y <- recycleVector(y, m)
  invisible(.Call("qt_qdrawPolygon_Painter", p, as.numeric(x), as.numeric(y),
                  .normArgColor(stroke, m), .normArgColor(fill, m)))
}

## Text drawing: a mess

## It seems that (at least in base R graphics) there are three
## different ways to align text: left/bottom, center, right/top.
## For horizontal alignment, it's pretty straight-forward. Italics
## might introduce some error (extending left of 0,0), but it's not a
## huge deal.

## For vertical alignment, there is a twist: the alignment can either
## be relative to the bounding box, or relative to the
## baseline. There are good use cases for both.

## Bounding box alignment is based on the text extents. If there are
## multiple lines, use the boundingRect(QRectF) method, otherwise
## tightBoundingRect. This is slow, but without it, the baseline
## effect would lead to misleading pictures.

## Baseline alignment uses only the ascent, and could be based on font
## metrics, or text extents (tightBoundingRect). R uses the font
## metrics, which is fastest. We will do the same.

## For multiple lines, each line is aligned the same as the block.

qdrawText <- function(p, text, x, y, halign = c("center", "left", "right"),
                      valign = c("center", "basecenter", "baseline", "bottom",
                        "top"),
                      rot = 0)
{
  m <- max(length(text), length(x), length(y))
  text <- recycleVector(text, m)
  x <- recycleVector(x, m)
  y <- recycleVector(y, m)
  drawText <- function(text)
    invisible(.Call("qt_qdrawText_Painter", p, as.character(text),
                    as.numeric(x), as.numeric(y), as.integer(hflag + vflag),
                    as.numeric(rot)))
  stopifnot(inherits(p, "Painter"))
  hflags <- c(left = 0x1, right = 0x2, center = 0x4)
  halign <- match.arg(halign)
  hflag <- hflags[halign]
  vflags <- c(top = 0x20, bottom = 0x40, center = 0x80)
  valign <- match.arg(valign)
  vflag <- vflags[valign]
  ## single lines should be vertically centered exactly
  if (valign == "center") {
    multi <- grepl("\n", text, fixed=TRUE)
    if (any(multi)) { ## draw the multilines immediately
      drawText(text[multi])
      text <- text[!multi]
      x <- x[!multi]
      y <- y[!multi]
    }
    vflag <- NA
  }
  if (is.na(vflag)) {
    vflag <- vflags["top"]
    ascent <- qfontMetrics(p)["ascent"]
    if (valign == "basecenter")
      ascent <- ascent / 2
    else if (valign == "center") {
      extents <- qtextExtents(p, text)
      y <- y - (extents[,"y1"] - extents[,"y0"]) / 2 + extents[,"y1"]
    }
    y <- y + ascent
  }
  drawText(text)
}

## qtext <- function(p, text, x, y, halign = c("center", "left", "right"),
##                    valign = c("center", "basecenter", "baseline", "bottom",
##                      "top"),
##                    rot = 0)
## {
##   stopifnot(inherits(p, "Painter"))
##   halign <- match.arg(halign)
##   valign <- match.arg(valign)
##   extents <- NULL

##   ## align vertically
##   if (valign != "baseline") {
##     if (valign == "center") {
##       extents <- qtextExtents(p, text, TRUE)
##       print(extents)
##       offset <- (extents[,"y1"] - extents[,"y0"]) / 2 - extents[,"y1"]
##     } else {
##       metrics <- qfontMetrics(p)
##       offset <- if (valign == "bottom")
##         -metrics["descent"]
##       else metrics["ascent"]
##       if (valign == "basecenter")
##         offset <- offset / 2
##     }
##     print(offset)
##     y <- y - offset
##   }
  
##   ## align horizontally
##   if (halign != "left") {
##     if (is.null(extents))
##       extents <- qtextExtents(p, text, valign == "center")
##     hadj <- if (halign == "center") 0.5 else 1
##     x <- x - hadj*(extents[,"x1"] - extents[,"x0"]) - extents[,"x0"]
##   }
  
##   print(c(x,y))
##   invisible(.Call("qt_qdrawText_Painter", p, as.character(text), as.numeric(x),
##                   as.numeric(y), as.numeric(rot)))
## }

qfontMetrics <- function(p) {
  stopifnot(inherits(p, "Painter"))
  ans <- .Call("qt_qfontMetrics_Painter", p)
  names(ans) <- c("ascent", "descent")
  ans
}

qtextExtents <- function(p, text) {
  stopifnot(inherits(p, "Painter"))
  ans <- .Call("qt_qtextExtents_Painter", p, as.character(text))
  colnames(ans) <- c("x0", "y0", "x1", "y1")
  ans
}

qstrWidth <- function(p, text) {
  ## FIXME: optimize by directly asking for widths, heights are expensive
  extents <- qtextExtents(p, text)
  extents[,3] - extents[,1]
}

qdrawImage <- function(p, col, width, height, x = 0, y = height-1) {
  stopifnot(inherits(p, "Painter"))
  stopifnot(length(col) == width*height)
  m <- max(length(x), length(y))
  x <- recycleVector(x, m)
  y <- recycleVector(y, m)
### FIXME: could also accept a matrix, like that returned here
  rgb <- as.raw(.normArgColor(col))
  invisible(.Call("qt_qdrawImage_Painter", p, rgb, as.integer(width),
                  as.integer(height)), as.numeric(x), as.numeric(y))
}

qdrawGlyph <- function(p, path, x, y, cex = NULL, stroke = NULL, fill = NULL) {
  stopifnot(inherits(p, "Painter"))
  stopifnot(inherits(path, "QPainterPath"))
  m <- max(length(x), length(y))
  x <- recycleVector(x, m)
  y <- recycleVector(y, m)
  if (!is.null(cex))
    cex <- recycleVector(as.numeric(cex), m)
  invisible(.Call("qt_qdrawGlyphs_Painter", p, path, as.numeric(x),
                  as.numeric(y), cex, .normArgColor(stroke, m),
                  .normArgColor(fill, m)))
}

qpath <- function() {
  invisible(.Call("qt_qpath"))
}


### NOTE: these are not vectorized, as it is probably not necessary

### IDEA: add grid graphical primitives with '+' function

qpathCircle <- function(x, y, r, path = qpath()) {
  stopifnot(inherits(path, "QPainterPath"))
  invisible(.Call("qt_qaddCircle_QPainterPath", path, as.numeric(x),
                  as.numeric(y), as.numeric(r)))
}
qpathRect <- function(xleft, ybottom, xright, ytop, path = qpath()) {
  stopifnot(inherits(path, "QPainterPath"))
  invisible(.Call("qt_qaddRect_QPainterPath", path, as.numeric(xleft),
                  as.numeric(ytop), as.numeric(xright - xleft),
                  as.numeric(ytop - ybottom)))
}
qpathPolygon <- function(x, y, path = qpath()) {
  stopifnot(inherits(path, "QPainterPath"))
  invisible(.Call("qt_qaddPolygon_QPainterPath", path, as.numeric(x),
                  as.numeric(y)))
}
qpathText <- function(text, x, y, font, path = qpath()) {
  stopifnot(inherits(path, "QPainterPath"))
  invisible(.Call("qt_qaddText_QPainterPath", path, as.character(text),
                  as.numeric(x), as.numeric(y), as.character(font$family),
                  as.integer(font$pointsize), as.integer(font$weight),
                  as.logical(font$italic)))
}
qpathSegment <- function(x0, y0, x1, y1, path = qpath()) {
  stopifnot(inherits(path, "QPainterPath"))
  invisible(.Call("qt_qaddSegment_QPainterPath", path, as.numeric(x0),
                  as.numeric(y0), as.numeric(x1), as.numeric(y1)))
}
