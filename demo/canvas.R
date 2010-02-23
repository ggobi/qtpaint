## tests/demonstrations for the interactive canvas
library(qtpaint)

options(warn=2)
options(error=recover)

circle <- qpathCircle(0, 0, 5)

n <- 1000
x <- rnorm(n, 50, 25)
y <- rnorm(n, 50, 25)
df <- data.frame(X = x, Y = y)

##data(mtcars)
##df <- mtcars[,c("mpg", "hp")]

##data(iris)
##df <- iris

fill <- col2rgb(rgb(1, seq(0, 1, length=nrow(df)), 0, 0.5), TRUE)
scatterplot <- function(item, painter, exposed) {
  ##qstrokeColor(painter) <- NA
  ##qfillColor(painter) <- fill
  ##qantialias(painter) <- FALSE
  ##qdrawPoint(painter, df[,1], df[,2], stroke = fill)
  ##qdrawGlyph(painter, circle, df[,1], df[,2], fill = fill)
  qdrawLine(painter, c(30, 60, 20, NA, 15, 50), c(40, 20, 80, NA, 30, 20))
  ##qdrawLine(painter, c(NA,NA), c(NA, NA))
   ## qcircle(painter, df[,1], df[,2], rep(5, n),
   ##          stroke = fill, fill = fill)
  ## qrect(painter, df[,1], df[,2], df[,1] + 2, df[,2] + 2)
}

labeled <- rep(FALSE, nrow(df))
labeler <- function(item, painter, exposed) {
  mat <- qdeviceMatrix(item, inverted=TRUE)
  off <- qmap(mat, c(5, 5)) - qmap(mat, c(0, 0))
  df <- df[labeled,]
  qdrawText(painter, rownames(df), df[,1]+off[1], df[,2]+off[2], "left",
            "bottom")
}

margin <- 5
adjust <- c(margin, -margin)

## axes <- function(item, painter, exposed) {
##   view <- qtpaint:::qpaintingView(item) # include in 'context'?
##   qfont(painter) <- qfont(pointsize=12)
##   pos <- qmap(qdeviceMatrix(item, view), qboundingRect(view) + adjust)
##   qtext(painter, colnames(df)[1], pos[2], pos[4], "right", "bottom")
##   qtext(painter, colnames(df)[2], pos[1], pos[3], "left", "top")
## }

adjustPoint <- Qt$QPointF(margin, margin)

axes <- function(item, painter, exposed) {
  qfont(painter) <- qfont(pointsize=12)
  ## geom <- item$geometry
  ## geom$setLowerRight(geom$lowerRight() - adjustPoint)
  ## geom$setBottomLeft(geom$upperLeft() + adjustPoint)
  ## qpainter$drawText(geom, colnames(df)[1], Qt$AlignRight | Qt$AlignBottom)
  ## qpainter$drawText(geom, colnames(df)[2], Qt$AlignLeft | Qt$AlignTop)
  pos <- qgeometry(item) + adjust
  qdrawText(painter, colnames(df)[1], pos[2], pos[4], "right", "bottom")
  qdrawText(painter, colnames(df)[2], pos[1], pos[3], "left", "top")
}

pointAdder <- function(event) {
  df <<- rbind(df, event$pos)
  qupdate(scene)
}

pointIdentifier <- function(event) {
  off <- 20
  offv <- c(-off, off)
  pos <- event$screenPos
  ## offPoint <- Qt$QPointF(off, off)
  ## rect <- Qt$QRectF(pos - offPoint, pos + offPoint)
  rect <- qrect(pos[1] + offv, pos[2] + offv)
  mat <- qdeviceMatrix(event$item, event$view)
  rect <- qmap(mat, rect)
  ## rect <- mat$mapRect(rect)
  hits <- qprimitives(event$item, rect)
  hitmat <- as.matrix(df[hits,])
  posmat <- matrix(event$pos, ncol=2)
  labeled <<- rep(FALSE, nrow(df))
  labeled[hits][Biobase::matchpt(posmat, hitmat)[,1]] <<- TRUE
  qupdate(labels)
}

boundsPainter <- function(item, painter, exposed) {
  lims <- qboundingRect(item)
  qstrokeColor(painter) <- "red"
  qdrawRect(painter, lims[1,1], lims[1,2], lims[2,1], lims[2,2])
}

scene <- qgraphicsScene()
##qbackground(scene) <- "black"
root <- qlayer(scene)
points <- qlayer(root, scatterplot, mouseMove = pointIdentifier)
qlimits(points) <- qrect(range(df[,1]), range(df[,2]))
labels <- qlayer(root, labeler)
qcacheMode(labels) <- "none"
qlimits(labels) <- qlimits(points)
##bounds <- qlayer(NULL, boundsPainter)
##qaddGraphicsWidget(root, bounds, 1, 1)
view <- qplotView(scene = scene, opengl = TRUE)
overlay <- qoverlay(view)
axesOverlay <- qlayer(overlay, axes)
print(view)
