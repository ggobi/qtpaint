## Performance testing

library(qtpaint)

ops <- c("glyph", "polygon", "rectangle", "point", "line", "segment", "circle")

test <- function(op, opengl, antialias) {
  op <- match(op, ops)
  .Call("qanviz_painter_test", op - 1L, as.logical(opengl),
        as.logical(antialias))
}

params <-
  expand.grid(antialias = c(TRUE, FALSE), opengl = c(TRUE, FALSE), op = ops)

for (i in seq(nrow(params))) do.call(test, params[i,])
