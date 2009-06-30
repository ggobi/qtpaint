### some utilities to help make drawing easier

qmap <- function(m, x, y) {
  cl <- NULL
  if (missing(y)) {
    cl <- class(x) # if only 'x' specified, preserve its class
    if (!is.matrix(x))
      x <- matrix(x, ncol = 2, byrow = TRUE)
    y <- x[,2]
    x <- x[,1]
  }
  mapped <- cbind(x * m[1,1] + y * m[2,1] + m[3,1],
                  y * m[2,2] + x * m[1,2] + m[3,2])
  class(mapped) <- cl
  mapped
}

qscale <- function(m = qmatrix(), s = c(x, y), x = 1, y = 1) {
  s <- rep(s, length = 2)
  m * matrix(c(rep(s, each = 2), 1, 1), 3, byrow=TRUE)
}

qtranslate <- function(m = qmatrix(), t = c(x, y), x = 0, y = 0) {
  s <- rep(t, length = 2)
  m + matrix(c(rep(0, 4), colSums(t * m[1:2,])), 3, byrow=TRUE)
}

qrotate <- function(m = qmatrix(), r = 0) {
  cs <- c(cos(rotate), sin(rotate))
  smat <- mat[1:2,]
  matrix(c(colSums(cs * smat), colSums(c(-cs[2], cs[1]) * smat), mat[3,]),
         3, byrow=TRUE)
}

.validRect <- function(r) {
  is.matrix(r) && is.numeric(r) && identical(dim(r), c(2L, 2L))
}

qflipY <- function(ymax, ymin = 0) {
  if (.validRect(ymax)) {
    ymin <- ymax[3]
    ymax <- ymax[4]
  }
  qtranslate(qscale(y = -1), y = -(ymax + ymin))
}
