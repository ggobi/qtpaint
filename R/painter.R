## R wrappers around the Qanviz::Painter API
## These are the lowest level wrappers

## should be called 'qtransform', but qtbase already uses that for a
## convenience constructor (would rather not make it generic)
qdeviceTransform <- function(x) {
  stopifnot(inherits(x, "Painter"))
  .Call("qt_qtransform_Painter", x)
}
`qdeviceTransform<-` <- function(x, value) {
  stopifnot(inherits(x, "Painter"))
  stopifnot(inherits(value, "QTransform"))
  .Call("qt_qsetTransform_Painter", x, value, PACKAGE = "qtpaint")
}

`qdeviceTransformEnabled<-` <- function(p, value) {
  stopifnot(inherits(p, "Painter"))
  invisible(.Call("qt_qsetTransformEnabled_Painter", p, as.logical(value)))
}

`qhasStroke<-` <- function(p, value) {
  stopifnot(inherits(p, "Painter"))
  invisible(.Call("qt_qsetHasStroke_Painter", p, as.logical(value)))
}

`qhasFill<-` <- function(p, value) {
  stopifnot(inherits(p, "Painter"))
  invisible(.Call("qt_qsetHasFill_Painter", p, as.logical(value)))
}

.normArgStroke <- function(p, color, len) {
  if (length(color) == 1) {
    qstrokeColor(p) <- .normArgColor(color)
    NULL
  } else .normArgColor(color, len)
}

.normArgFill <- function(p, color, len) {
  if (length(color) == 1) {
    qfillColor(p) <- .normArgColor(color)
    NULL
  } else .normArgColor(color, len)
}

.normArgColor <- function(color, len) {
  if (is.null(color))
    return(NULL)
  if (inherits(color, "QColor"))
    color <- as.matrix(color) # simplifies C code
  if (!is.matrix(color) || !is.integer(color) || nrow(color) != 4)
    color <- col2rgb(color, TRUE)
  if (!missing(len)) # might drop to vector here, much faster, C code is OK
    color <- recycleVector(color, 4L*len)
  color
}

`qstrokeColor<-` <- function(x, value) {
  stopifnot(inherits(x, "Painter"))
  color <- .normArgColor(value)
  invisible(.Call("qt_qsetStrokeColor_Painter", x, color))
}

`qfillColor<-` <- function(x, value) {
  stopifnot(inherits(x, "Painter"))
  color <- .normArgColor(value)
  invisible(.Call("qt_qsetFillColor_Painter", x, color))
}

## 'value' is a QFont
`qfont<-` <- function(x, value)
{
  stopifnot(inherits(x, "Painter"))
  stopifnot(inherits(value, "QFont"))
  invisible(.Call("qt_qsetFont_Painter", x, value))
}

`qlineWidth<-` <- function(x, value) {
  stopifnot(inherits(x, "Painter"))
  invisible(.Call("qt_qsetLineWidth_Painter", x, as.integer(value)))
}

`qdash<-` <- function(x, value) {
  stopifnot(inherits(x, "Painter"))
  invisible(.Call("qt_qsetDashes_Painter", x, as.numeric(value)))
}

`qantialias<-` <- function(x, value) {
  stopifnot(inherits(x, "Painter"))
  invisible(.Call("qt_qsetAntialias_Painter", x, as.logical(value)))
}

qdrawLine <- function(p, x, y, stroke = NULL) {
  stopifnot(inherits(p, "Painter"))
  m <- max(length(x), length(y))
  x <- recycleVector(x, m)
  y <- recycleVector(y, m)
  invisible(.Call("qt_qdrawPolyline_Painter", p, as.numeric(x), as.numeric(y),
                  .normArgStroke(p, stroke, m)))
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
                  .normArgStroke(p, stroke, m)))
}

qdrawPoint <- function(p, x, y, stroke = NULL) {
  stopifnot(inherits(p, "Painter"))
  m <- max(length(x), length(y))
  x <- recycleVector(x, m)
  y <- recycleVector(y, m)
  invisible(.Call("qt_qdrawPoints_Painter", p, as.numeric(x), as.numeric(y),
                  .normArgStroke(p, stroke, m)))
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
                  as.numeric(ytop - ybottom), .normArgStroke(p, stroke, m),
                  .normArgFill(p, fill, m)))
}

qdrawCircle <- function(p, x, y, r, stroke = NULL, fill = NULL) {
  stopifnot(inherits(p, "Painter"))
  m <- max(length(x), length(y), length(r))
  x <- recycleVector(x, m)
  y <- recycleVector(y, m)
  r <- recycleVector(r, m)
  invisible(.Call("qt_qdrawCircle_Painter", p, as.numeric(x), as.numeric(y),
                  as.integer(r), .normArgStroke(p, stroke, m),
                  .normArgFill(p, fill, m)))
}

qdrawPolygon <- function(p, x, y, stroke = NULL, fill = NULL) {
  stopifnot(inherits(p, "Painter"))
  m <- max(length(x), length(y))
  x <- recycleVector(x, m)
  y <- recycleVector(y, m)
  invisible(.Call("qt_qdrawPolygon_Painter", p, as.numeric(x), as.numeric(y),
                  .normArgStroke(p, stroke, m), .normArgFill(p, fill, m)))
}

qdrawPath <- function(p, path, stroke = NULL, fill = NULL) {
  stopifnot(inherits(p, "Painter"))
  if (inherits(path, "QPainterPath"))
    path <- list(path)
  else path <- as.list(path)
  m <- length(path)
  invisible(.Call("qt_qdrawPath_Painter", p, path,
                  .normArgStroke(p, stroke, m), .normArgFill(p, fill, m)))
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
                      rot = rep.int(0, length(text)), color = NULL)
{
  m <- max(length(text), length(x), length(y))
  text <- recycleVector(text, m)
  x <- recycleVector(x, m)
  y <- recycleVector(y, m)
  rot <- recycleVector(rot, m)
  drawText <- function(text)
    invisible(.Call("qt_qdrawText_Painter", p, as.character(text),
                    as.numeric(x), as.numeric(y), as.integer(hflag + vflag),
                    as.numeric(rot), .normArgStroke(p, color, m)))
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
    adj <- 0
    if (valign == "basecenter")
      ascent <- ascent / 2
    else if (valign == "center") {
      extents <- qtextExtents(p, text)
      adj <- -(extents[,"y1"] - extents[,"y0"]) / 2
    }
    adj <- adj + ascent
    ## fix adjustment for rotation
    rads <- rot/360*2*pi
    tf <- qdeviceTransform(p)
    ## we perform an "inverse" rotation in Y, map Y to pixels, then back to X
    ## 'adj' is a magnitude, so we have to subtract the origin (0)
    ## this works around the flipped Y axis
    mapToX <- function(y)
      qmap(tf$inverted(), qmap(tf, 0, sin(rads) * y)[,2], 0)[,1]
    x <- x + mapToX(adj) - mapToX(0)
    y <- y + cos(rads)*adj
  }
  drawText(text)
}

qfontMetrics <- function(p) {
  stopifnot(inherits(p, "Painter"))
  ans <- .Call("qt_qfontMetrics_Painter", p)
  names(ans) <- c("ascent", "descent")
  ans
}

qtextExtents <- function(x, text) {
  ans <- .Call("qt_qtextExtents_Painter", x, as.character(text))
  colnames(ans) <- c("x0", "y0", "x1", "y1")
  ans
}

qstrWidth <- function(p, text) {
  ## FIXME: optimize by directly asking for widths, heights are expensive
  extents <- qtextExtents(p, text)
  extents[,3] - extents[,1]
}

qstrHeight <- function(p, text) {
    extents <- qtextExtents(p, text)
    extents[,4] - extents[,2]
}

qdrawImage <- function(p, image, x, y) {
  stopifnot(inherits(p, "Painter"))
  stopifnot(inherits(image, "QImage"))
  m <- max(length(x), length(y))
  x <- recycleVector(x, m)
  y <- recycleVector(y, m)
  invisible(.Call("qt_qdrawImage_Painter", p, image, as.numeric(x),
                  as.numeric(y)))
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
                  as.numeric(y), cex, .normArgStroke(p, stroke, m),
                  .normArgFill(p, fill, m)))
}
