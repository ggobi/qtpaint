\name{transform}
\alias{qdeviceTransform}
\title{User to device transform}
\usage{qdeviceTransform(x)
qdeviceTransform(x) <- value
qdeviceTransformEnabled(x) <- value}
\description{Access the user to device coordinate transform of the paint context.}
\value{\code{qdeviceTransform}: The user to device transform, as a C++ \code{QTransform} object

}
\author{Michael Lawrence}
\arguments{\item{x}{The paint context}
\item{value}{The desired user to device transform, as a C++
\code{QTransform} object, or a logical scalar for
\code{qdeviceTransformEnabled}.}
}
\alias{qdeviceTransform<-}
\alias{qdeviceTransformEnabled<-}

