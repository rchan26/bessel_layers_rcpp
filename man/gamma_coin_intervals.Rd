% Generated by roxygen2: do not edit by hand
% Please edit documentation in R/RcppExports.R
\name{gamma_coin_intervals}
\alias{gamma_coin_intervals}
\title{Gamma coin flipper for intervals}
\usage{
gamma_coin_intervals(u, k, X, times, l, v)
}
\arguments{
\item{u}{simulated value from random U[0,1]}

\item{k}{integer value starting index for calculating the intervals}

\item{X}{vector of values of Brownian bridge}

\item{times}{vector of times}

\item{l}{lower bound of Brownian bridge}

\item{v}{upper bound of Brownian bridge}
}
\value{
boolean value: if T, accept probability that Brownian bridge remains 
        in [l,v], otherwise reject
}
\description{
Flips 'Gamma coin' for intervals; takes the product of the Cauchy sequence S^{gamma}_{k} to
determine whether or not the Brownian bridge remains in the interval [l,v]
}
\examples{
# setting up Brownian bridge variable
brownian_bridge <- matrix(c(0, 0, -0.2, 0.4, 0.3, 0.5, 1, 1),
                          ncol = 4, nrow = 2)

# flip delta coin whether or not Brownian bridge remains in [-0.5, 1.5]
gamma_coin_intervals(u = runif(1, 0, 1),
                     k = 1,
                     X = brownian_bridge[1,],
                     times = brownian_bridge[2,],
                     l = -0.5,
                     v = 1.5)

}
