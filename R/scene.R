## convenience wrapper for constructor

qscene <- function(parent = NULL) Qt$QGraphicsScene(parent)

## vectorized scene manipulation functions from qtgui

qscene.points <- function(s, x, y, radius = 1)
{
  xy <- xy.coords(x, y, recycle = TRUE)
  .Call(scene_addPoints, s, as.double(xy$x), as.double(xy$y), as.double(radius))
}

qscene.lines <- function(s, x, y, lwd = 0)
{
  xy <- xy.coords(x, y, recycle = TRUE)
  .Call(scene_addLines, s, as.double(xy$x), as.double(xy$y), as.double(lwd))
}

qscene.segments <- function(s, x1, y1, x2, y2, lwd = 0)
{
  n <- max(length(x1), length(x2), length(y1), length(y2))
  .Call(scene_addSegments,
        s,
        rep(as.double(x1), length.out = n),
        rep(as.double(y1), length.out = n),
        rep(as.double(x2), length.out = n),
        rep(as.double(y2), length.out = n),
        as.double(lwd))
}

qscene.rect <- function(s, x, y, w = 1, h = 1)
{
  xy <- xy.coords(x, y, recycle = TRUE)
  .Call(scene_addRect, s, as.double(xy$x), as.double(xy$y), as.double(w),
        as.double(h))
}

qsetItemFlags <- function(x, flag = c("movable", "selectable"), status = FALSE)
{
  flag <- match.arg(flag)
  .Call(qt_qsetItemFlags, x, flag, status)
}

qsetTextItemInteraction <- function(x, mode = c("none", "editor", "browser"))
{
  mode <- match.arg(mode)
  .Call(qt_qsetTextItemInteraction, x, mode)
}
