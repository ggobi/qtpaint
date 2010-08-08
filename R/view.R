## convenient R-level constructor, following Layer's precedent
qplotView <- function(scene, parent = NULL,
                      rescale = c("geometry", "transform", "none"),
                      opengl = TRUE)
{
  rescale <- c(none = 0L, geometry = 1L, transform = 2L)[match.arg(rescale)]
  view <- Qanviz$PlotView(scene, parent, rescale, opengl)
  if (is.null(scene$parent())) # view becomes default parent of scene
    scene$setParent(view)
  view
}
