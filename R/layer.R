## Low level wrappers around (R)Layer
  
qvLayer <- function(parent = NULL, paintFun = NULL, keyPressFun = NULL,
                    keyReleaseFun = NULL, mouseDoubleClickFun = NULL,
                    mouseMoveFun = NULL, mousePressFun = NULL,
                    mouseReleaseFun = NULL, wheelFun = NULL)
{
  layer <- .Call("newRLayer",
                 .normArgCallback(paintFun),
                 .normArgCallback(keyPressFun),
                 .normArgCallback(keyReleaseFun),
                 .normArgCallback(mouseDoubleClickFun),
                 .normArgCallback(mouseMoveFun),
                 .normArgCallback(mousePressFun),
                 .normArgCallback(mouseReleaseFun),
                 .normArgCallback(wheelFun))
  if (inherits(parent, "QViz::Layer")) {
    qvAddLayer(parent, layer, 0, 0, 1, 1)
  } else if (inherits(parent, "QGraphicsScene"))
    qvAddLayer(parent, layer)
  else if (!is.null(parent)) stop("Unsupported parent type")
  qvCacheMode(layer) <- "device"
  layer
}

.normArgCallback <- function(callback) {
  if (!is.null(callback))
    callback <- as.function(callback)
  callback
}

qvSetLimits <- function(p, xlim, ylim) {
  stopifnot(inherits(p, "QViz::Layer"))
  if (is.matrix(xlim)) {
    ylim <- xlim[,2]
    xlim <- xlim[,1]
  }
  stopifnot(length(xlim) == 2)
  stopifnot(length(ylim) == 2)
  invisible(.Call("Layer_setLimits", p, as.numeric(xlim), as.numeric(ylim)))
}

qvLimits <- function(p) {
  stopifnot(inherits(p, "QViz::Layer"))
  .Call("Layer_limits", p)
}

test <- function(p, s) {
  .Call("test", p, s$extp)
}

### NOTE: really only makes sense if layer is top-level
qvSetGeometry <- function(p, x0, y0, x1, y1) {
### FIXME: technically QGraphicsLayoutItem, but that's not a QObject
  stopifnot(inherits(p, "QGraphicsWidget"))
  x <- x0
  if (!is.matrix(x)) {
    x <- matrix(c(as.numeric(x0), as.numeric(x1),
                  as.numeric(y0), as.numeric(y1)),
                2, 2)
  } else stopifnot(is.numeric(x) && ncol(x) == 2 && nrow(x) == 2)
  invisible(.Call("QGraphicsWidget_setGeometry", p, x))
}

qvGeometry <- function(p) {
  stopifnot(inherits(p, "QGraphicsWidget"))
  .Call("QGraphicsWidget_geometry", p)
}

qvBoundingRect <- function(p) {
  if (inherits(p, "QGraphicsWidget"))
    .Call("QGraphicsItem_boundingRect", p)
  else if (inherits(p, "QGraphicsView"))
    .Call("QGraphicsView_viewportRect", p)
  else stop("Invalid parameter 'p'")
}

qvPaintingView <- function(p) {
  stopifnot(inherits(p, "QGraphicsWidget"))
  .Call("QGraphicsItem_paintingView", p)
}

qvDeviceMatrix <- function(p, view = qvPaintingView(p), inverted = TRUE) {
  stopifnot(inherits(p, "QGraphicsWidget"))
  stopifnot(inherits(view, "QGraphicsView"))
  .Call("QGraphicsItem_deviceMatrix", p, view, as.logical(inverted))
}

qvOverlay <- function(p) {
  if (!inherits(p, "QViz::PlotView"))
    p <- p$extp
  stopifnot(inherits(p, "QViz::PlotView"))
  .Call("PlotView_overlay", p)
}

qvAddGraphicsWidget <- function(p, child, row = 0, col = 0, nrow = 1, ncol = 1)
{
  stopifnot(inherits(child, "QGraphicsWidget"))
  if (inherits(p, "QGraphicsScene")) {
    invisible(.Call("QGraphicsScene_addWidget", p, child))
  } else {
    stopifnot(inherits(p, "QViz::Layer"))
    invisible(.Call("Layer_addChildItem", p, child, as.integer(row),
                    as.integer(col), as.integer(nrow), as.integer(ncol)))
  }
}
qvAddLayer <- qvAddGraphicsWidget

qvRowStretch <- function(p) {
  stopifnot(inherits(p, "QViz::Layer"))
  .Call("Layer_rowStretch", p)
}

qvColStretch <- function(p) {
  stopifnot(inherits(p, "QViz::Layer"))
  .Call("Layer_colStretch", p)
}

`qvRowStretch<-` <- function(p, value) {
  stopifnot(inherits(p, "QViz::Layer"))
  invisible(.Call("Layer_setRowStretch", p, as.integer(value)))
}

`qvColStretch<-` <- function(p, value) {
  stopifnot(inherits(p, "QViz::Layer"))
  invisible(.Call("Layer_setColStretch", p, as.integer(value)))
}

`qvHSpacing<-` <- function(p, value) {
  stopifnot(inherits(p, "QViz::Layer"))
  invisible(.Call("Layer_setHorizontalSpacing", p, as.numeric(value)))
}

`qvVSpacing<-` <- function(p, value) {
  stopifnot(inherits(p, "QViz::Layer"))
  invisible(.Call("Layer_setVerticalSpacing", p, as.numeric(value)))
}

"[<-.Layer" <-
  function (x, i, j, ..., value)
{
  qvAddGraphicsWidget(x, value, row = i, col = j, ...)
}

qvUpdate <- function(p) {
  if (inherits(p, "QGraphicsScene"))
    invisible(.Call("QGraphicsScene_update", p))
  else if (inherits(p, "QGraphicsWidget"))
    invisible(.Call("QGraphicsItem_update", p))
  else stop("Invalid parameter 'p'")
}

### FIXME: seems this would benefit from some formal classes
qvItems <- function(p, x, y) {
  stopifnot(inherits(p, "QViz::Layer"))
  if (is.matrix(x)) {
    stopifnot(is.numeric(x) && ncol(x) == 2 && nrow(x) == 2)
    .Call("Layer_itemsInRect", p, x)
  } else if (is.numeric(x)) {
    if (length(x) == 2)
      .Call("Layer_itemsAtPoint", p, x[1], x[2])
    else {
      y <- as.numeric(y)
      if (length(x) == 1)
        .Call("Layer_itemsAtPoint", p, x, y)
      else .Call("Layer_itemsInPolygon", p, x, y)
    }
  } else if (inherits(x, "QPainterPath"))
    .Call("Layer_itemsInPath", p, x)
  else stop("invalid arguments")
}

`qvCacheMode<-` <- function(x, value) {
  stopifnot(inherits(x, "QGraphicsWidget"))
  modes <- c(none = 0L, item = 1L, device = 2L)
  mode <- modes[value]
  if (is.na(mode))
    stop("'value' must be one of ", paste(names(modes), collapse = ", "))
  invisible(.Call("QGraphicsItem_setCacheMode", x, mode))
}

qvCacheMode <- function(x) {
  stopifnot(inherits(x, "QGraphicsWidget"))
  modes <- c("none", "item", "device")
  modes[.Call("QGraphicsItem_cacheMode", x, PACKAGE="qtpaint") + 1]
}

qvFocus <- function(x) {
  stopifnot(inherits(x, "QGraphicsWidget"))
  .Call("QGraphicsItem_setFocus", x, PACKAGE="qtpaint")
}

.boundingDim <- function(x) {
  r <- qvBoundingRect(x)
  r[2,] - r[1,]
}

dim.QGraphicsItem <- .boundingDim
dim.QGraphicsView <- .boundingDim

`qvBackground<-` <- function(x, value) {
  stopifnot(inherits(x, "QGraphicsScene"))
  .Call("QGraphicsScene_setBackground", x, .normArgColor(value),
        PACKAGE="qtpaint")
}
