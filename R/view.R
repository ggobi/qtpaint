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
  stopifnot(inherits(x, "QGraphicsView"))
  .Call("qt_qqGraphicsView_qt_qsetMatrix", x, value, PACKAGE = "qtpaint")
}
