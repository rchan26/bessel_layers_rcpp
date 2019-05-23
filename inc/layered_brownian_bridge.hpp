#ifndef LAYERED_BROWNIAN_BRIDGE
#define LAYERED_BROWNIAN_BRIDGE

#include "brownian_bridge_min.hpp"
#include "inverse_gauss.hpp"
#include "cauchy_sums.hpp"
#include "coins_cauchy.hpp"
#include "bessel_layer_sim.hpp"

using namespace Rcpp;

// forward declaration for function that finds the maximum value in a vector
double find_max(const Rcpp::NumericVector vect);

// forward declaration for function that finds the minimum value in a vector
double find_min(const Rcpp::NumericVector vect);

// forward declaration layared brownian bridge sampler
Rcpp::NumericMatrix layered_brownian_bridge(const double &x, const double &y,
                                            const double &s, const double &t,
                                            const Rcpp::NumericVector &a, const int &l,
                                            Rcpp::NumericVector &times);

#endif