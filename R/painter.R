## R wrappers around the QViz::Painter API
## These are the lowest level wrappers

## TODO: maybe move to a model where 'p' is obtained from a hidden
## variable in the parent (callback) frame.

qvMatrix <- function(p, inverted = FALSE) {
  if (inherits(p, "Painter"))
    invisible(.Call("Painter_matrix", p, as.logical(inverted)))
  else if (inherits(p, "QGraphicsWidget")) # obviously need S3 dispatching
    invisible(.Call("QGraphicsItem_matrix", p, as.logical(inverted)))
  else stop("argument 'p' has unknown type")
}

`qvMatrixEnabled<-` <- function(p, value) {
  stopifnot(inherits(p, "Painter"))
  invisible(.Call("Painter_setMatrixEnabled", p, as.logical(value)))
}

`qvHasStroke<-` <- function(p, value) {
  stopifnot(inherits(p, "Painter"))
  invisible(.Call("Painter_setHasStroke", p, as.logical(value)))
}

`qvHasFill<-` <- function(p, value) {
  stopifnot(inherits(p, "Painter"))
  invisible(.Call("Painter_setHasFill", p, as.logical(value)))
}

.normArgColor <- function(color) {
  if (is.matrix(color) && is.integer(color) && nrow(color) == 4)
    color
  else col2rgb(color, TRUE)
}

`qvStrokeColor<-` <- function(p, value) {
  stopifnot(inherits(p, "Painter"))
  color <- .normArgColor(value)
  invisible(.Call("Painter_setStrokeColor", p, color))
}

`qvFillColor<-` <- function(p, value) {
  stopifnot(inherits(p, "Painter"))
  color <- .normArgColor(value)
  invisible(.Call("Painter_setFillColor", p, color))
}

## 'font' as returned by 'qfont'
`qvFont<-` <- function(p, value)
{
  stopifnot(inherits(p, "Painter"))
  invisible(.Call("Painter_setFont", p, as.character(value$family),
                  as.integer(value$pointsize), as.integer(value$weight),
                  as.logical(value$italic)))
}

`qvLineWidth<-` <- function(p, value) {
  stopifnot(inherits(p, "Painter"))
  invisible(.Call("Painter_setLineWidth", p, as.integer(value)))
}

`qvDash<-` <- function(p, value) {
  stopifnot(inherits(p, "Painter"))
  invisible(.Call("Painter_setDashes", p, as.numeric(value)))
}

`qvAntialias<-` <- function(p, value) {
  stopifnot(inherits(p, "Painter"))
  invisible(.Call("Painter_setAntialias", p, as.logical(value)))
}

qvPolyline <- function(p, x, y, stroke = NULL) {
  stopifnot(inherits(p, "Painter"))
  invisible(.Call("Painter_drawPolyline", p, as.numeric(x), as.numeric(y),
                  .normArgColor(stroke)))
}

qvSegment <- function(p, x0, y0, x1, y1, stroke = NULL) {
  stopifnot(inherits(p, "Painter"))
  invisible(.Call("Painter_drawSegments", p, as.numeric(x0), as.numeric(y0),
                  as.numeric(x1), as.numeric(y1),
                  .normArgColor(stroke)))
}

qvPoint <- function(p, x, y, stroke = NULL) {
  stopifnot(inherits(p, "Painter"))
  invisible(.Call("Painter_drawPoints", p, as.numeric(x), as.numeric(y),
                  .normArgColor(stroke)))
}

qvRect <- function(p, xleft, ybottom, xright, ytop, stroke = NULL, fill = NULL)
{
  stopifnot(inherits(p, "Painter"))
  invisible(.Call("Painter_drawRectangles", p, as.numeric(xleft),
                  as.numeric(ybottom), as.numeric(xright - xleft),
                  as.numeric(ytop - ybottom), .normArgColor(stroke),
                  .normArgColor(fill)))
}

qvCircle <- function(p, x, y, r, stroke = NULL, fill = NULL) {
  stopifnot(inherits(p, "Painter"))
  invisible(.Call("Painter_drawCircle", p, as.numeric(x), as.numeric(y),
                  as.integer(r), .normArgColor(stroke), .normArgColor(fill)))
}

qvPolygon <- function(p, x, y, stroke = NULL, fill = NULL) {
  stopifnot(inherits(p, "Painter"))
  invisible(.Call("Painter_drawPolygon", p, as.numeric(x), as.numeric(y),
                  .normArgColor(stroke), .normArgColor(fill)))
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

qvText <- function(p, text, x, y, halign = c("center", "left", "right"),
                   valign = c("center", "basecenter", "baseline", "bottom",
                     "top"),
                   rot = 0)
{
  drawText <- function(text)
    invisible(.Call("Painter_drawText", p, as.character(text), as.numeric(x),
                    as.numeric(y), as.integer(hflag + vflag), as.numeric(rot)))
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
    ascent <- qvFontMetrics(p)["ascent"]
    if (valign == "basecenter")
      ascent <- ascent / 2
    else if (valign == "center") {
      extents <- qvTextExtents(p, text)
      y <- y - (extents[,"y1"] - extents[,"y0"]) / 2 + extents[,"y1"]
    }
    y <- y + ascent
  }
  drawText(text)
}

## qvText <- function(p, text, x, y, halign = c("center", "left", "right"),
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
##       extents <- qvTextExtents(p, text, TRUE)
##       print(extents)
##       offset <- (extents[,"y1"] - extents[,"y0"]) / 2 - extents[,"y1"]
##     } else {
##       metrics <- qvFontMetrics(p)
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
##       extents <- qvTextExtents(p, text, valign == "center")
##     hadj <- if (halign == "center") 0.5 else 1
##     x <- x - hadj*(extents[,"x1"] - extents[,"x0"]) - extents[,"x0"]
##   }
  
##   print(c(x,y))
##   invisible(.Call("Painter_drawText", p, as.character(text), as.numeric(x),
##                   as.numeric(y), as.numeric(rot)))
## }

qvFontMetrics <- function(p) {
  stopifnot(inherits(p, "Painter"))
  ans <- .Call("Painter_fontMetrics", p)
  names(ans) <- c("ascent", "descent")
  ans
}

qvTextExtents <- function(p, text) {
  stopifnot(inherits(p, "Painter"))
  ans <- .Call("Painter_textExtents", p, as.character(text))
  colnames(ans) <- c("x0", "y0", "x1", "y1")
  ans
}

qvStrWidth <- function(p, text) {
  ## FIXME: optimize by directly asking for widths, heights are expensive
  extents <- qvTextExtents(p, text)
  extents[,3] - extents[,1]
}

qvImage <- function(p, col, width, height, x = 0, y = height-1) {
  stopifnot(inherits(p, "Painter"))
  stopifnot(length(col) == width*height)
### FIXME: could also accept a matrix, like that returned here
  rgb <- as.raw(.normArgColor(col))
  invisible(.Call("Painter_drawImage", p, rgb, as.integer(width),
                  as.integer(height)), as.numeric(x), as.numeric(y))
}

qvGlyph <- function(p, path, x, y, cex = NULL, stroke = NULL, fill = NULL) {
  stopifnot(inherits(p, "Painter"))
  stopifnot(inherits(path, "QPainterPath"))
  if (!is.null(cex))
    cex <- as.numeric(cex)
  invisible(.Call("Painter_drawGlyphs", p, path, as.numeric(x), as.numeric(y),
                  cex, .normArgColor(stroke), .normArgColor(fill)))
}

qvPath <- function() {
  invisible(.Call("newQPainterPath"))
}


### NOTE: these are not vectorized, as it is probably not necessary

### IDEA: add grid graphical primitives with '+' function

qvPathCircle <- function(x, y, r, path = qvPath()) {
  stopifnot(inherits(path, "QPainterPath"))
  invisible(.Call("QPainterPath_addCircle", path, as.numeric(x), as.numeric(y),
                  as.numeric(r)))
}
qvPathRect <- function(xleft, ybottom, xright, ytop, path = qvPath()) {
  stopifnot(inherits(path, "QPainterPath"))
  invisible(.Call("QPainterPath_addRect", path, as.numeric(xleft),
                  as.numeric(ytop), as.numeric(xright - xleft),
                  as.numeric(ytop - ybottom)))
}
qvPathPolygon <- function(x, y, path = qvPath()) {
  stopifnot(inherits(path, "QPainterPath"))
  invisible(.Call("QPainterPath_addPolygon", path, as.numeric(x),
                  as.numeric(y)))
}
qvPathText <- function(text, x, y, font, path = qvPath()) {
  stopifnot(inherits(path, "QPainterPath"))
  invisible(.Call("QPainterPath_addText", path, as.character(text),
                  as.numeric(x), as.numeric(y), as.character(font$family),
                  as.integer(font$pointsize), as.integer(font$weight),
                  as.logical(font$italic)))
}
qvPathSegment <- function(x0, y0, x1, y1, path = qvPath()) {
  stopifnot(inherits(path, "QPainterPath"))
  invisible(.Call("QPainterPath_addSegment", path, as.numeric(x0),
                  as.numeric(y0), as.numeric(x1), as.numeric(y1)))
}
