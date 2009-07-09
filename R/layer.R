## Low level wrappers around (R)Layer
  
qlayer <- function(parent = NULL, paintFun = NULL, keyPressFun = NULL,
                   keyReleaseFun = NULL, mouseDoubleClickFun = NULL,
                   mouseMoveFun = NULL, mousePressFun = NULL,
                   mouseReleaseFun = NULL, wheelFun = NULL, sizeHintFun = NULL,
                   geometry = qrect(0, 0, 600, 400))
{
  layer <- .Call("qt_qlayer",
                 .normArgCallback(paintFun),
                 .normArgCallback(keyPressFun),
                 .normArgCallback(keyReleaseFun),
                 .normArgCallback(mouseDoubleClickFun),
                 .normArgCallback(mouseMoveFun),
                 .normArgCallback(mousePressFun),
                 .normArgCallback(mouseReleaseFun),
                 .normArgCallback(wheelFun),
                 .normArgCallback(sizeHintFun))
  if (inherits(parent, "QViz::Layer")) {
    qaddItem(parent, layer, 0, 0, 1, 1)
  } else if (inherits(parent, "QGraphicsScene"))
    qaddItem(parent, layer)
  else if (!is.null(parent)) stop("Unsupported parent type")
  qgeometry(layer) <- geometry
  qcacheMode(layer) <- "device"
  qminimumSize(layer) <- qsize(1, 1) # so layout works
  layer
}

.normArgCallback <- function(callback) {
  if (!is.null(callback))
    callback <- as.function(callback)
  callback
}

`qlimits<-` <- function(x, value) {
  stopifnot(inherits(x, "QViz::Layer"))
  stopifnot(inherits(value, "QRectF"))
  invisible(.Call("qt_qsetLimits_Layer", x, value))
}

qlimits <- function(x) {
  stopifnot(inherits(x, "QViz::Layer"))
  .Call("qt_qlimits_Layer", x)
}

test <- function(p, s) {
  .Call("test", p, s$extp)
}

## technically, viewport rect, not bounding rect
### REMOVE when able to get viewport from the view
qboundingRect.QGraphicsView <- function(x) {
  .Call("qt_qviewportRect_QGraphicsView", x)
}

qpaintingView <- function(p) {
  stopifnot(inherits(p, "QGraphicsWidget"))
  .Call("qt_qpaintingView_QGraphicsItem", p)
}

qdeviceMatrix <- function(p, view = qpaintingView(p), inverted = TRUE) {
  stopifnot(inherits(p, "QGraphicsWidget"))
  stopifnot(inherits(view, "QGraphicsView"))
  .Call("qt_qdeviceMatrix_QGraphicsItem", p, view, as.logical(inverted))
}

qoverlay <- function(p) {
  if (!inherits(p, "QViz::PlotView"))
    p <- p$extp
  stopifnot(inherits(p, "QViz::PlotView"))
  .Call("qt_qoverlay_PlotView", p)
}

`qaddItem.QViz::Layer` <-
  function(x, item, row = 0, col = 0, nrow = 1, ncol = 1)
{
  invisible(.Call("qt_qaddItem_Layer", x, item, as.integer(row),
                  as.integer(col), as.integer(nrow), as.integer(ncol)))
}

qrowStretch <- function(p) {
  stopifnot(inherits(p, "QViz::Layer"))
  .Call("qt_qrowStretch_Layer", p)
}

qcolStretch <- function(p) {
  stopifnot(inherits(p, "QViz::Layer"))
  .Call("qt_qcolStretch_Layer", p)
}

`qrowStretch<-` <- function(x, value) {
  stopifnot(inherits(x, "QViz::Layer"))
  invisible(.Call("qt_qsetRowStretch_Layer", x, as.integer(value)))
}

`qcolStretch<-` <- function(x, value) {
  stopifnot(inherits(x, "QViz::Layer"))
  invisible(.Call("qt_qsetColStretch_Layer", x, as.integer(value)))
}

`qhSpacing<-` <- function(x, value) {
  stopifnot(inherits(x, "QViz::Layer"))
  invisible(.Call("qt_qsetHorizontalSpacing_Layer", x, as.numeric(value)))
}

`qvSpacing<-` <- function(x, value) {
  stopifnot(inherits(p, "QViz::Layer"))
  invisible(.Call("qt_qsetVerticalSpacing_Layer", x, as.numeric(value)))
}

"[<-.QViz::Layer" <-
  function (x, i, j, ..., value)
{
  qaddItem(x, value, row = i, col = j, ...)
}

qprimitives <- function(x, r) {
  if (is(r, "QRectF"))
    .Call("qt_qprimitivesInRect_Layer", x, r)
  else if (is(r, "QPointF"))
    .Call("qt_qprimitivesAtPoint_Layer", x, r)
  ## else if (is(r, "QPolygonF"))
  ##   .Call("qt_qprimitivesInPolygon_Layer", x, r)
  ## else if (is(r, "QPainterPath"))
  ##   .Call("qt_qprimitivesInPath_Layer", x, x)
  else stop("invalid arguments")  
}

`qbackgroundColor<-` <- function(x, value) {
  stopifnot(inherits(x, "QGraphicsScene"))
  .Call("qt_qsetBackgroundColor_QGraphicsScene", x, .normArgColor(value),
        PACKAGE="qtpaint")
}
