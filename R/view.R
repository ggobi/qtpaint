## convenient R-level constructor, following Layer's precedent
qplotView <- function(scene, parent = NULL,
                      rescale = c("geometry", "transform", "none"),
                      opengl = TRUE)
{
  rescale <- c(none = 0L, geometry = 1L, transform = 2L)[match.arg(rescale)]
  Qanviz$PlotView(scene, parent, rescale, opengl)
}
