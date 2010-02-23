## Conveniences for interacting with Qanviz::Layer.

## argument matching is positional in C++, which would make this
## difficult via Smoke, so we implement manually and add a few
## conveniences along the way
qlayer <- function(parent = NULL, paintFun = NULL, keyPressFun = NULL,
                   keyReleaseFun = NULL, mouseDoubleClickFun = NULL,
                   mouseMoveFun = NULL, mousePressFun = NULL,
                   mouseReleaseFun = NULL, wheelFun = NULL,
                   hoverMoveEvent = NULL, hoverEnterEvent = NULL,
                   hoverLeaveEvent = NULL, contextMenuEvent = NULL,
                   dragEnterEvent = NULL, dragLeaveEvent = NULL,
                   dragMoveEvent = NULL, dropEvent = NULL,
                   focusInEvent = NULL, focusOutEvent = NULL,
                   sizeHintFun = NULL,
                   geometry = qrect(0, 0, 600, 400), clip = TRUE)
{
  p <- NULL
  if (inherits(parent, "QGraphicsItem"))
    p <- parent
  layer <- Qanviz$RLayer(p,
                         .normArgCallback(paintFun),
                         .normArgCallback(keyPressFun),
                         .normArgCallback(keyReleaseFun),
                         .normArgCallback(mouseDoubleClickFun),
                         .normArgCallback(mouseMoveFun),
                         .normArgCallback(mousePressFun),
                         .normArgCallback(mouseReleaseFun),
                         .normArgCallback(wheelFun),
                         .normArgCallback(hoverMoveEvent),
                         .normArgCallback(hoverEnterEvent),
                         .normArgCallback(hoverLeaveEvent),
                         .normArgCallback(contextMenuEvent),
                         .normArgCallback(dragEnterEvent),
                         .normArgCallback(dragLeaveEvent),
                         .normArgCallback(dragMoveEvent),
                         .normArgCallback(dropEvent),
                         .normArgCallback(focusInEvent),
                         .normArgCallback(focusOutEvent),               
                         .normArgCallback(sizeHintFun))
  if (inherits(parent, "Qanviz::Layer")) {
    parent$addLayer(layer, 0, 0, 1, 1)
  } else if (inherits(parent, "QGraphicsScene"))
    parent$addItem(layer)
  else if (!is.null(parent)) stop("Unsupported parent type")
  layer$geometry <- qrect(geometry)
  layer$setFlag(Qt$QGraphicsItem$ItemClipsToShape, clip)
  layer
}

.normArgCallback <- function(callback) {
  if (!is.null(callback))
    callback <- as.function(callback)
  callback
}

## matrix-style syntax for accessing child layers
## the [,] operator does not exist in C++
"[<-.Qanviz::Layer" <-
  function (x, i, j, ..., value)
{
  x$addLayer(value, i, j, ...)
}
"[.Qanviz::Layer" <-
  function (x, i, j, ...)
{
  x$layout()$itemAt(i, j)
}
