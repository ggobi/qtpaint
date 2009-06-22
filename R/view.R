qvView <- function(w, scene = qvScene(w), fitScene = TRUE,
                   rescale = c("geometry", "transform", "none"),
                   opengl = TRUE)
{
  rescale <- c(none = 0L, geometry = 1L, transform = 2L)[match.arg(rescale)]
  e <- .Call("newViewWidget", scene, rescale, as.logical(opengl),
             PACKAGE = "qtpaint")
  if (fitScene)
    qvFitScene(e)
  e
}

qvFitScene <- function(x) {
  x <- if (inherits(x, "qwidget")) x$extp else x
  stopifnot(inherits(x, "QGraphicsView"))
  .Call("QGraphicsView_fitScene", x)
}


`qmatrix<-` <- function(x, value) {
  stopifnot(inherits(x, "QGraphicsView"))
  .Call("qt_qsetMatrix_QGraphicsView", x, value, PACKAGE = "qtpaint")
}
