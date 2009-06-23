## Low level wrappers around (R)Layer
  
qlayer <- function(parent = NULL, paintFun = NULL, keyPressFun = NULL,
                   keyReleaseFun = NULL, mouseDoubleClickFun = NULL,
                   mouseMoveFun = NULL, mousePressFun = NULL,
                   mouseReleaseFun = NULL, wheelFun = NULL)
{
  layer <- .Call("qt_qlayer",
                 .normArgCallback(paintFun),
                 .normArgCallback(keyPressFun),
                 .normArgCallback(keyReleaseFun),
                 .normArgCallback(mouseDoubleClickFun),
                 .normArgCallback(mouseMoveFun),
                 .normArgCallback(mousePressFun),
                 .normArgCallback(mouseReleaseFun),
                 .normArgCallback(wheelFun))
  if (inherits(parent, "QViz::Layer")) {
    qaddLayer(parent, layer, 0, 0, 1, 1)
  } else if (inherits(parent, "QGraphicsScene"))
    qaddLayer(parent, layer)
  else if (!is.null(parent)) stop("Unsupported parent type")
  qcacheMode(layer) <- "device"
  layer
}

.normArgCallback <- function(callback) {
  if (!is.null(callback))
    callback <- as.function(callback)
  callback
}

qsetLimits <- function(p, xlim, ylim) {
  stopifnot(inherits(p, "QViz::Layer"))
  if (is.matrix(xlim)) {
    ylim <- xlim[,2]
    xlim <- xlim[,1]
  }
  stopifnot(length(xlim) == 2)
  stopifnot(length(ylim) == 2)
  invisible(.Call("qt_qsetLimits_Layer", p, as.numeric(xlim), as.numeric(ylim)))
}

qlimits <- function(p) {
  stopifnot(inherits(p, "QViz::Layer"))
  .Call("qt_qlimits_Layer", p)
}

test <- function(p, s) {
  .Call("test", p, s$extp)
}

### NOTE: really only makes sense if layer is top-level
qsetGeometry <- function(p, x0, y0, x1, y1) {
### FIXME: technically QGraphicsLayoutItem, but that's not a QObject
  stopifnot(inherits(p, "QGraphicsWidget"))
  x <- x0
  if (!is.matrix(x)) {
    x <- matrix(c(as.numeric(x0), as.numeric(x1),
                  as.numeric(y0), as.numeric(y1)),
                2, 2)
  } else stopifnot(is.numeric(x) && ncol(x) == 2 && nrow(x) == 2)
  invisible(.Call("qt_qsetGeometry_QGraphicsWidget", p, x))
}

qgeometry <- function(p) {
  stopifnot(inherits(p, "QGraphicsWidget"))
  .Call("qt_qgeometry_QGraphicsWidget", p)
}

qboundingRect <- function(p) {
  if (inherits(p, "QGraphicsWidget"))
    .Call("qt_qboundingRect_QGraphicsItem", p)
  else if (inherits(p, "QGraphicsView"))
    .Call("qt_qviewportRect_QGraphicsView", p)
  else stop("Invalid parameter 'p'")
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

qaddGraphicsWidget <- function(p, child, row = 0, col = 0, nrow = 1, ncol = 1)
{
  stopifnot(inherits(child, "QGraphicsWidget"))
  if (inherits(p, "QGraphicsScene")) {
    invisible(.Call("qt_qaddWidget_QGraphicsScene", p, child))
  } else {
    stopifnot(inherits(p, "QViz::Layer"))
    invisible(.Call("qt_qaddChildItem_Layer", p, child, as.integer(row),
                    as.integer(col), as.integer(nrow), as.integer(ncol)))
  }
}
qaddLayer <- qaddGraphicsWidget

qrowStretch <- function(p) {
  stopifnot(inherits(p, "QViz::Layer"))
  .Call("qt_qrowStretch_Layer", p)
}

qcolStretch <- function(p) {
  stopifnot(inherits(p, "QViz::Layer"))
  .Call("qt_qcolStretch_Layer", p)
}

`qrowStretch<-` <- function(p, value) {
  stopifnot(inherits(p, "QViz::Layer"))
  invisible(.Call("qt_qsetRowStretch_Layer", p, as.integer(value)))
}

`qcolStretch<-` <- function(p, value) {
  stopifnot(inherits(p, "QViz::Layer"))
  invisible(.Call("qt_qsetColStretch_Layer", p, as.integer(value)))
}

`qhSpacing<-` <- function(p, value) {
  stopifnot(inherits(p, "QViz::Layer"))
  invisible(.Call("qt_qsetHorizontalSpacing_Layer", p, as.numeric(value)))
}

`qvSpacing<-` <- function(p, value) {
  stopifnot(inherits(p, "QViz::Layer"))
  invisible(.Call("qt_qsetVerticalSpacing_Layer", p, as.numeric(value)))
}

"[<-.Layer" <-
  function (x, i, j, ..., value)
{
  qaddGraphicsWidget(x, value, row = i, col = j, ...)
}

qupdate <- function(p) {
  if (inherits(p, "QGraphicsScene"))
    invisible(.Call("qt_qupdate_QGraphicsScene", p))
  else if (inherits(p, "QGraphicsWidget"))
    invisible(.Call("qt_qupdate_QGraphicsItem", p))
  else stop("Invalid parameter 'p'")
}

### FIXME: seems this would benefit from some formal classes
qitems <- function(p, x, y) {
  stopifnot(inherits(p, "QViz::Layer"))
  if (is.matrix(x)) {
    stopifnot(is.numeric(x) && ncol(x) == 2 && nrow(x) == 2)
    .Call("qt_qitemsInRect_Layer", p, x)
  } else if (is.numeric(x)) {
    if (length(x) == 2)
      .Call("qt_qitemsAtPoint_Layer", p, x[1], x[2])
    else {
      y <- as.numeric(y)
      if (length(x) == 1)
        .Call("qt_qitemsAtPoint_Layer", p, x, y)
      else .Call("qt_qitemsInPolygon_Layer", p, x, y)
    }
  } else if (inherits(x, "QPainterPath"))
    .Call("qt_qitemsInPath_Layer", p, x)
  else stop("invalid arguments")
}

`qcacheMode<-` <- function(x, value) {
  stopifnot(inherits(x, "QGraphicsWidget"))
  modes <- c(none = 0L, item = 1L, device = 2L)
  mode <- modes[value]
  if (is.na(mode))
    stop("'value' must be one of ", paste(names(modes), collapse = ", "))
  invisible(.Call("qt_qsetCacheMode_QGraphicsItem", x, mode))
}

qcacheMode <- function(x) {
  stopifnot(inherits(x, "QGraphicsWidget"))
  modes <- c("none", "item", "device")
  modes[.Call("qt_qcacheMode_QGraphicsItem", x, PACKAGE="qtpaint") + 1]
}

qfocus <- function(x) {
  stopifnot(inherits(x, "QGraphicsWidget"))
  .Call("qt_qsetFocus_QGraphicsItem", x, PACKAGE="qtpaint")
}

.boundingDim <- function(x) {
  r <- qboundingRect(x)
  r[2,] - r[1,]
}

dim.QGraphicsItem <- .boundingDim
dim.QGraphicsView <- .boundingDim

`qbackground<-` <- function(x, value) {
  stopifnot(inherits(x, "QGraphicsScene"))
  .Call("qt_qsetBackground_QGraphicsScene", x, .normArgColor(value),
        PACKAGE="qtpaint")
}
