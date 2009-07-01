qplotView <- function(scene, rescale = c("geometry", "transform", "none"),
                      opengl = TRUE)
{
  rescale <- c(none = 0L, geometry = 1L, transform = 2L)[match.arg(rescale)]
  e <- .Call("qt_qplotView", scene, rescale, as.logical(opengl),
             PACKAGE = "qtpaint")
  qfitScene(e)
  e
}

dim.QGraphicsView <- function(x) dim(qboundingRect(x))

`qopengl<-` <- function(x, value) {
  .Call("qt_qsetOpengl", x, value)
}
