\name{item-properties}
\alias{qsetItemFlags}
\title{Item properties}
\usage{qsetItemFlags(x, flag=Qt$QGraphicsItem$ItemIsMovable, status=FALSE)
qsetTextItemInteraction(x, mode=c("none", "editor", "browser"))
}
\description{Set properties of all items in a scene. Fast path for when a scene
contains many items.}
\author{Deepayan Sarkar and Michael Lawrence}
\arguments{\item{x}{The scene, a C++ \code{QGraphicsScene}}
\item{flag}{A value or combination of values from the
\code{QGraphicsItem::GraphicsItemFlag} enumeration}
\item{status}{Whether the flag should be set to \code{TRUE} or \code{FALSE}}
\item{mode}{Whether a text item (C++ \code{QGraphicsTextItem})
should behave as a text editor, text browser, or not allow
interaction.}
}
\alias{qsetTextItemInteraction}

