\name{gCat}
\alias{gCat}
\title{Graph-based two-sample tests for categorical data}
\description{These are two-sample tests for categorical data utilizing similarity information among the categories.  They are useful when there is underlying structure on the categories.}
\author{
Hao Chen and Nancy R. Zhang

Maintainer: Hao Chen (hxchen@ucdavis.edu)
}
\references{
Chen, H. and Zhang, N.R. (2013) Graph-based tests for two-sample comparisons of categorical data. Statistica Sinica, 23, 1479-1503.
}
\seealso{
  \code{\link{gcat.test}}
  }
\examples{
data(Example)
gcat.test(mycounts,mydist)
}

