\name{qlayer}
\alias{qlayer}
\title{Create a layer}
\usage{qlayer(parent, paintFun, keyPressFun, keyReleaseFun, mouseDoubleClickFun,
    mouseMoveFun, mousePressFun, mouseReleaseFun, wheelFun,
    hoverMoveFun, hoverEnterFun, hoverLeaveFun, contextMenuFun,
    dragEnterFun, dragLeaveFun, dragMoveFun, dropFun, focusInFun,
    focusOutFun, sizeHintFun, limits=qrect(), row=0, col=0, rowSpan=1,
    colSpan=1, geometry=qrect(0, 0, 600, 400), clip=cache, cache=FALSE)
}
\description{This function constructs a Layer, an item in the canvas that may
paint part of the plot and/or respond to user input. The behavior
of the layer is implemented using R functions, which are passed to
the constructor. Other arguments determine the scaling and
positioning of the layer, and whether the drawing of the layer is
cached and/or clipped.}
\details{All drawing and user input handling is performed by R callbacks,
which must accept a specific set of arguments. The paint callback,
passed as \code{paintFun}, must take at least two arguments,
conventionally named \code{layer} and \code{painter}. The
\code{layer} argument is a C++ \code{RLayer} object, the same
instance that was created by calling the constructor. All painting
is performed through the \code{painter} argument, which is a C++
\code{Painter} object. See the \code{\link[=qdrawGlyph]{paint functions}}
for more details. The \code{paintFun} may take one additional, optional
argument, conventionally named \code{exposed}, which is the rectangle,
in layer coordinates, that needs to be drawn.

All of the other callbacks, except for \code{sizeHintFun}, are
event handlers. Two arguments are passed, conventionally named
\code{layer} and \code{event}. The former is the layer constructed
in the call to \code{qlayer}, and the latter describes the event
as an instance of C++ \code{QGraphicsSceneEvent}. The exact
subclass depends on the event. Manipulating an event currently
requires low-level calls through the \code{qtbase} package. See
its documentation.}
\value{The layer, a C++ instance of \code{RLayer}}
\author{Michael Lawrence}
\arguments{\item{parent}{The scene, for a top-level layer, or the parent
layer that contains the new layer in a grid layout}
\item{paintFun}{The function that implements painting, called
whenever the layer needs to be repainted. All drawing occurs in
data/layer coordinates.}
\item{keyPressFun}{The function called when a key is pressed}
\item{keyReleaseFun}{The function called when a key is released}
\item{mouseDoubleClickFun}{The function called when a mouse button
is double-clicked}
\item{mouseMoveFun}{The function called when the mouse is moved
while holding down a button}
\item{mousePressFun}{The function called when a mouse button is pressed}
\item{mouseReleaseFun}{The function called when a mouse button is released}
\item{wheelFun}{The function called when the mouse wheel is turned}
\item{hoverMoveFun}{The function called when the mouse moves
without any buttons pressed}
\item{hoverEnterFun}{The function called when the mouse pointer
enters the layer}
\item{hoverLeaveFun}{The function called when the mouse pointer
leaves the layer}
\item{contextMenuFun}{The function called when a context menu is
requested, such as through a right mouse button click}
\item{dragEnterFun}{The function called when the pointer enters
the layer while dragging something}
\item{dragLeaveFun}{The function called when the pointer leaves
the layer while dragging something}
\item{dragMoveFun}{The function called when the pointer moves within
the layer while dragging something}
\item{dropFun}{The function called when something is dropped on the layer}
\item{focusInFun}{The function called when the layer gains the
keyboard focus}
\item{focusOutFun}{The function called when the layer loses the
keyboard focus}
\item{sizeHintFun}{The function called to determine the size
constraints of the layer. Not currently documented.}
\item{limits}{A \code{QRectF}, possibly created by
\code{\link[qtbase]{qrect}}, indicating the X and Y scales of the
layer in data/layer coordinates}
\item{row}{The 0-based row index of the layer in the parent grid layout}
\item{col}{The 0-based column index of the layer in the parent grid layout}
\item{rowSpan}{The 0-based number of rows spanned by the layer in the layout}
\item{colSpan}{The 0-based number of cols spanned by the layer in the layout}
\item{geometry}{A \code{QRectF}, possibly created by
\code{\link[qtbase]{qrect}}, indicating the position and size of
the layer in figure/scene coordinates. This is overridden by the
parent grid layout, so is really only useful for a top-level layer.}
\item{clip}{Logical indicating whether to clip drawing to within the layer}
\item{cache}{Logical indicating whether to cache drawing, which
helps performance for relatively static layers sitting under more
dynamic ones}
}
\examples{scene <- qscene()
layer <- qlayer(scene, function(layer, painter) {
qdrawCircle(1:10, 1:10)
}, limits = qtbase::qrect(0, 0, 11, 11))
qplotView(scene)}

