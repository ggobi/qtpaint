### some utilities to help make drawing easier

## Some simple glyph generators

qglyphCircle <- function(r = 5) {
  glyph <- Qt$QPainterPath()
  glyph$addEllipse(qpoint(0, 0), r, r)
  glyph
}

qglyphSquare <- function(x = 5) {
  glyph <- Qt$QPainterPath()
  glyph$addRect(-x, -x, 2*x, 2*x)
  glyph
}

qglyphTriangle <- function(x = 5, direction = c("up", "down")) {
  direction <- match.arg(direction)
  if (direction == "down")
    x <- -x
  glyph <- Qt$QPainterPath()
  glyph$moveTo(-x, x)
  glyph$lineTo(x, x)
  glyph$lineTo(0, -x)
  glyph$closeSubpath()
  glyph
}

##' Create a segment glyph with specified length and slope
##'
##' Create a segment glyph with specified length and slope
##' @title Create a segment glyph with specified length and slope
##' @param x length
##' @param b slope
##' @return a glyph
##' @author Yihui Xie <\url{http://yihui.name}>
qglyphSegment <- function(x = 5, b = 0) {
  glyph <- Qt$QPainterPath()
  x0 <- x * cos(atan(b))
  y0 <- x * sin(atan(b))
  glyph$moveTo(-x0, -y0)
  glyph$lineTo(x0, y0)
  glyph
}

## not quite sure about this
qglyphText <- function(text = "X", size = 12) {
  glyph <- Qt$QPainterPath()
  glyph$addText(-size / 2, size / 2, qfont(pointsize = size), text)
  glyph
}

## Need a fast way to map data. Easiest to take QTransform as an R
## matrix and use vectorized arithmetic.
qmap <- function(m, x, y) {
  m <- as.matrix(m)
  cl <- NULL
  if (missing(y)) {
    cl <- class(x) # if only 'x' specified, preserve its class
    if (NCOL(x) == 1L) {
      tmpX <- try(as.numeric(x), silent=TRUE)
      if (!is(tmpX, "try-error"))
        x <- matrix(tmpX, ncol = 2, byrow = TRUE)
      else {
        x <- try(as.matrix(x), silent=TRUE)
        if (is(x, "try-error"))
          stop("if 'y' is missing, 'x' must be coercible to vector or matrix")
      }
    }
    y <- x[,2]
    x <- x[,1]
  }
  mapped <- cbind(x * m[1,1] + y * m[2,1] + m[3,1],
                  y * m[2,2] + x * m[1,2] + m[3,2])
  if (!is.null(cl) && canCoerce(mapped, cl))
    mapped <- as(mapped, cl)
  mapped
}

## Creates a QTransform that flips the Y axis

.validRect <- function(r) {
  is.matrix(r) && is.numeric(r) && identical(dim(r), c(2L, 2L))
}

qflipY <- function(ymax, ymin = 0) UseMethod("qflipY")
qflipY.numeric <- function(ymax, ymin = 0) {
  if (.validRect(ymax)) {
    ymin <- ymax[3]
    ymax <- ymax[4]
  }
  Qt$QTransform(1, 0, 0, -1, 0, (ymax + ymin))
}
qflipY.QRect <- qflipY.QRectF <-
  function(ymax, ymin = 0) qflipY(as.matrix(ymax))

## Getting dimensions of rectangles and rectangular objects

dim.QRectF <- function(x) c(x$width(), x$height())

dim.QGraphicsScene <- function(x) dim(x$sceneRect)

dim.QGraphicsItem <- function(x) dim(x$boundingRect)

dim.QGraphicsView <- function(x) dim(x$viewport()$rect)

## The main purpose for this qupdate() is to refresh the item cache
## But isn't this a bug in Qt?
## Should make sure this problem still exists in Qt 4.6.

qupdate <- function(x) UseMethod("qupdate")

qupdate.QGraphicsView <- function(x) {
  qupdate(x$scene())
  x$viewport()$repaint()
}

refreshItemCache <- function(item) {
  mode <- item$cacheMode()
  item$setCacheMode(Qt$QGraphicsItem$NoCache)
  item$setCacheMode(mode)
}

qupdate.QGraphicsScene <- function(x) {
  lapply(x$items(), refreshItemCache)
  x$update()
}

qupdate.QGraphicsItem <- function(x) {
  refreshItemCache(x)
  x$update()
}
