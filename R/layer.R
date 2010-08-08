## Conveniences for interacting with Qanviz::Layer.

## argument matching is positional in C++, which would make this
## difficult via Smoke, so we implement manually and add a few
## conveniences along the way
qlayer <- function(parent = NULL, paintFun = NULL, keyPressFun = NULL,
                   keyReleaseFun = NULL, mouseDoubleClickFun = NULL,
                   mouseMoveFun = NULL, mousePressFun = NULL,
                   mouseReleaseFun = NULL, wheelFun = NULL,
                   hoverMoveFun = NULL, hoverEnterFun = NULL,
                   hoverLeaveFun = NULL, contextMenuFun = NULL,
                   dragEnterFun = NULL, dragLeaveFun = NULL,
                   dragMoveFun = NULL, dropFun = NULL,
                   focusInFun = NULL, focusOutFun = NULL,
                   sizeHintFun = NULL, limits = qrect(),
                   row = 0L, col = 0L, rowSpan = 1L, colSpan = 1L,
                   geometry = qrect(0, 0, 600, 400), clip = TRUE, cache = TRUE)
{
  p <- NULL
  if (inherits(parent, "QGraphicsItem"))
    p <- parent
  args <- list(p,
               .normArgCallback(paintFun),
               .normArgCallback(keyPressFun),
               .normArgCallback(keyReleaseFun),
               .normArgCallback(mouseDoubleClickFun),
               .normArgCallback(mouseMoveFun),
               .normArgCallback(mousePressFun),
               .normArgCallback(mouseReleaseFun),
               .normArgCallback(wheelFun),
               .normArgCallback(hoverMoveFun),
               .normArgCallback(hoverEnterFun),
               .normArgCallback(hoverLeaveFun),
               .normArgCallback(contextMenuFun),
               .normArgCallback(dragEnterFun),
               .normArgCallback(dragLeaveFun),
               .normArgCallback(dragMoveFun),
               .normArgCallback(dropFun),
               .normArgCallback(focusInFun),
               .normArgCallback(focusOutFun),               
               .normArgCallback(sizeHintFun))
  layer <- .Call("qanviz_RLayer", args, PACKAGE="qtpaint")
  if (inherits(parent, "Qanviz::Layer")) {
    parent$addLayer(layer, row, col, rowSpan, colSpan)
  } else if (inherits(parent, "QGraphicsScene"))
    parent$addItem(layer)
  else if (!is.null(parent)) stop("Unsupported parent type")
  layer$geometry <- geometry
  layer$setLimits(limits)
  layer$setFlag(Qt$QGraphicsItem$ItemClipsToShape, clip)
  if (!cache)
    layer$setCacheMode(Qt$QGraphicsItem$NoCache)
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

## wrappers that could be added by anyone interested in maintaining them:

## qlimits <- function(x) x$limits()
## "qlimits<-" <- function(x, limits) x$setLimits(limits)
## qaddLayer <- function(x, child) x$addLayer(child)
## qdeviceTransform: layer$deviceTransform(event)
## q(row,col)Stretch[<-]: layer$layout()$(row,col)[Set]Stretch()
## q(h,v)Spacing[<-]: layer$layout()$setHorizontal(Vertical)Spacing()
## qbackgroundBrush[<-]: scene$backgroundBrush <- brush
## qclearSelection: layer$clearSelection()
## qzValue[<-]: layer$setZValue()
## qlocate: layer$locate()
## qminimumSize[<-]: layer$setMinimumSize()
## qcacheMode[<-]: layer$setCacheMode()
## qclip[<-]: layer$setClip()
## qfocus[<-]: layer$setFocus()
## qoverlayScene: layer$overlayScene()
