qview <- function(w, scene = qscene(w), fitScene = TRUE,
                   rescale = c("geometry", "transform", "none"),
                   opengl = TRUE)
{
  rescale <- c(none = 0L, geometry = 1L, transform = 2L)[match.arg(rescale)]
  e <- .Call("qt_qview", scene, rescale, as.logical(opengl),
             PACKAGE = "qtpaint")
  if (fitScene)
    qfitScene(e)
  e
}

qfitScene <- function(x) {
  x <- if (inherits(x, "qwidget")) x$extp else x
  stopifnot(inherits(x, "QGraphicsView"))
  .Call("qt_qfitScene_QGraphicsView", x)
}

`qmatrix<-` <- function(x, value) {
  if (inherits(x, "QGraphicsView"))
    .Call("qt_qsetMatrix_QGraphicsView", x, value, PACKAGE = "qtpaint")
  else if (inherits(x, "QGraphicsWidget"))
    .Call("qt_qsetMatrix_QGraphicsItem", x, value, PACKAGE = "qtpaint")
  else if (inherits(x, "QPainter"))
    .Call("qt_qsetMatrix_QPainter", x, value, PACKAGE = "qtpaint")
  else stop("unknown type of 'x'")
}
