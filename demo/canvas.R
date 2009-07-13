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
  qstrokeColor(painter) <- NA
  ##qfillColor(painter) <- fill
  ##qantialias(painter) <- FALSE
  ##qdrawPoint(painter, df[,1], df[,2], stroke = fill)
  qdrawGlyph(painter, circle, df[,1], df[,2], fill = fill)
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

axes <- function(item, painter, exposed) {
  qfont(painter) <- qfont(pointsize=12)
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
  rect <- qrect(pos[1] + offv, pos[2] + offv)
  mat <- qdeviceMatrix(event$item, event$view)
  rect <- qmap(mat, rect)
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



##view2 <- qview(scene = scene)
##print(view2)

## for (i in 1:100) {
##    df <- df + 0.01
##    qtpaint:::qupdate(scene)
## }

## text alignment fun
## qstrokeColor(painter) <- rgb(0, 0, 0, 1)
## qsegment(painter, -100, 50, 200, 50)
## qfont(painter) <- q.font(pointsize=12)
## qtext(painter, "qgp", 50, 50, valign = "baseline")
## qtext(painter, "X", 60, 50, "left", "baseline")
## qstrokeColor(painter) <- rgb(0, 0, 1, 1)
## qtext(painter, "qgp", 50, 50, valign = "basecenter")
## qtext(painter, "X", 60, 50, "left", "basecenter")
## qstrokeColor(painter) <- rgb(0, 1, 0, 1)

## qtext(painter, "qgp", 50, 50)
## qtext(painter, "foo\nbar", 30, 50, "right", "center")
## qtext(painter, "X", 60, 50, "left")
## qtext(painter, "X", 195, -95, "right", "bottom")
## qtext(painter, "q", 175, -95, "right", "bottom")
## qtext(painter, "Y", -95, 195, "left", "top")


library(qtpaint)
circle <- qpathCircle(0, 0, 5)

n <- 1000
x <- rnorm(n, 50, 25)
y <- rnorm(n, 50, 25)
df <- data.frame(X = x, Y = y)

fill <- col2rgb(rgb(1, seq(0, 1, length=nrow(df)), 0, 0.5), TRUE)
scatterplot <- function(item, painter, exposed) {
  qstrokeColor(painter) <- NA
  qdrawGlyph(painter, circle, df[,1], df[,2], fill = fill)
}
xaxisFun <- function(item, painter, exposed) {
    at <- pretty(exposed[, 1])
    qdrawText(painter, as.character(at), at, 0.5)
}

scene <- qgraphicsScene()
root <- qlayer(scene)

points <- qlayer(NULL, scatterplot)
qlimits(points) <- qrect(range(df[,1]), range(df[,2]))

xaxis <- qlayer(NULL, xaxisFun)
qlimits(xaxis) <- qrect(range(df[,1]), c(0, 1))

## either of the following works, but not both

qaddItem(root, points, 0, 0)
qaddItem(root, xaxis, 1, 0)

view <- qplotView(scene = scene, opengl = FALSE)
print(view)

qminimumSize(xaxis) <- qsize(1, 20)

qrowStretch(xaxis) <- 0







