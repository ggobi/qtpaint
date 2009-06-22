### some utilities to help make drawing easier

qvMap <- function(m, x, y) {
  if (missing(y)) {
    if (!is.matrix(x))
      x <- matrix(x, ncol = 2, byrow = TRUE)
    y <- x[,2]
    x <- x[,1]
  }
  cbind(x * m[1,1] + y * m[2,1] + m[3,1],
        y * m[2,2] + x * m[1,2] + m[3,2])
}
