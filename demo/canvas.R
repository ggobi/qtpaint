## tests/demonstrations for the interactive canvas
library(qtpaint)

circle <- qvPathCircle(0, 0, 5)

n <- 100000
x <- rnorm(n, 50, 25)
y <- rnorm(n, 50, 25)
df <- data.frame(X = x, Y = y)

##data(mtcars)
##df <- mtcars[,c("mpg", "hp")]

##data(iris)
##df <- iris

fill <- col2rgb(rgb(1, seq(0, 1, length=nrow(df)), 0, 1), TRUE)

scatterplot <- function(item, painter, exposed) {
  ## qvFillColor(painter) <- "red"
  ##qvPoint(painter, df[,1], df[,2], stroke = fill)
  qvGlyph(painter, circle, df[,1], df[,2], stroke = fill, fill = fill)
   ## qvCircle(painter, df[,1], df[,2], rep(5, n),
   ##          stroke = fill, fill = fill)
  ## qvRect(painter, df[,1], df[,2], df[,1] + 2, df[,2] + 2)
}

labeled <- rep(FALSE, nrow(df))
labeler <- function(item, painter, exposed) {
  mat <- qvDeviceMatrix(item, inverted=TRUE)
  off <- qvMap(mat, c(5, 5)) - qvMap(mat, c(0, 0))
  df <- df[labeled,]
  qvText(painter, rownames(df), df[,1]+off[1], df[,2]+off[2], "left", "bottom")
}

margin <- 5
adjust <- c(margin, -margin)

## axes <- function(item, painter, exposed) {
##   view <- qtpaint:::qvPaintingView(item) # include in 'context'?
##   qvFont(painter) <- qfont(pointsize=12)
##   pos <- qvMap(qvDeviceMatrix(item, view), qvBoundingRect(view) + adjust)
##   qvText(painter, colnames(df)[1], pos[2], pos[4], "right", "bottom")
##   qvText(painter, colnames(df)[2], pos[1], pos[3], "left", "top")
## }

axes <- function(item, painter, exposed) {
  qvFont(painter) <- qfont(pointsize=12)
  pos <- qvGeometry(item) + adjust
  qvText(painter, colnames(df)[1], pos[2], pos[4], "right", "bottom")
  qvText(painter, colnames(df)[2], pos[1], pos[3], "left", "top")
}

pointAdder <- function(event) {
  df <<- rbind(df, event$pos)
  qvUpdate(scene)
}

pointIdentifier <- function(event) {
  off <- 20
  rect <- matrix(event$screenPos, 2, 2, byrow=TRUE) + rep(c(-off, off), 2)
  mat <- qvDeviceMatrix(event$item, event$view)
  rect <- qvMap(mat, rect)
  hits <- qvItems(event$item, rect)
  hitmat <- as.matrix(df[hits,])
  posmat <- matrix(event$pos, ncol=2)
  labeled <<- rep(FALSE, nrow(df))
  labeled[hits][Biobase::matchpt(posmat, hitmat)[,1]] <<- TRUE
  qvUpdate(labels)
}

boundsPainter <- function(item, painter, exposed) {
  lims <- qvBoundingRect(item)
  qvStrokeColor(painter) <- "red"
  qvRect(painter, lims[1,1], lims[1,2], lims[2,1], lims[2,2])
}

scene <- qvScene()
##qvBackground(scene) <- "black"
root <- qvLayer(scene)
points <- qvLayer(root, scatterplot, mouseMove = pointIdentifier)
qvSetLimits(points, range(df[,1]), range(df[,2]))
labels <- qvLayer(root, labeler)
qvCacheMode(labels) <- "none"
qvSetLimits(labels, qvLimits(points))
##bounds <- qvLayer(boundsPainter)
##qvAddGraphicsWidget(root, bounds, 1, 1)
view <- qvView(scene = scene, opengl = TRUE)
overlay <- qvOverlay(view)
axesOverlay <- qvLayer(overlay, axes)
print(view)

##view2 <- qvView(scene = scene)
##print(view2)

## for (i in 1:100) {
##    df <- df + 0.01
##    qtpaint:::qvUpdate(scene)
## }

## text alignment fun
## qvStrokeColor(painter) <- rgb(0, 0, 0, 1)
## qvSegment(painter, -100, 50, 200, 50)
## qvFont(painter) <- qv.font(pointsize=12)
## qvText(painter, "qgp", 50, 50, valign = "baseline")
## qvText(painter, "X", 60, 50, "left", "baseline")
## qvStrokeColor(painter) <- rgb(0, 0, 1, 1)
## qvText(painter, "qgp", 50, 50, valign = "basecenter")
## qvText(painter, "X", 60, 50, "left", "basecenter")
## qvStrokeColor(painter) <- rgb(0, 1, 0, 1)

## qvText(painter, "qgp", 50, 50)
## qvText(painter, "foo\nbar", 30, 50, "right", "center")
## qvText(painter, "X", 60, 50, "left")
## qvText(painter, "X", 195, -95, "right", "bottom")
## qvText(painter, "q", 175, -95, "right", "bottom")
## qvText(painter, "Y", -95, 195, "left", "top")
