#include "../inc/layered_brownian_bridge.hpp"

using namespace Rcpp;

// [[Rcpp::plugins("cpp17")]]

// [[Rcpp::export]]
double find_max(const Rcpp::NumericVector vect) 
{
	// finds the maximum value in a vector
    double current_max = vect.at(0); // first element
    for (const auto &element: vect) {
        if (element > current_max) {
            current_max = element;
        }
    }
    return current_max;
}

// [[Rcpp::export]]
double find_min(const Rcpp::NumericVector vect)
{
	// finds the minimum value in a vector
    double current_min = vect.at(0); // first element
    for (const auto &element: vect) {
        if (element < current_min) {
            current_min = element;
        }
    }
    return current_min;
}

// [[Rcpp::export]]
Rcpp::NumericMatrix layered_brownian_bridge(const double &x, const double &y,
                                            const double &s, const double &t,
                                            const Rcpp::NumericVector &a, const int &l,
                                            Rcpp::NumericVector &times)
{
  // layered Brownian bridge sampler
  // simulate from uniform random variable
  double u = Rcpp::runif(1, 0.0, 1.0)[0];
  
  while (true) {
    // declare variables required
    double l1, l2, v1, v2;
    Rcpp::NumericMatrix simulated_BB;
    Rcpp::NumericVector bridge_times = times;
    
    if (u < 0.5) {
      // simulate minimum point
      Rcpp::NumericVector simulated_min;
      
      // setting l1, l2, v1, v2 values in the different cases
      // simulating minimum points if needed
      if (l==0 && a.at(0)==0) {
        // simulated Bessel layer was [min(x,y), max(x,y)]
        // set minimum to occur at min(x,y)
        if (x < y) {
          // mimimum occurs at (x) at time (s)
          l1 = x, l2 = x;
          v1 = y, v2 = y;
          simulated_min = Rcpp::NumericVector::create(x, s);
        } else {
          // minimum occurs at (y) at time (t)
          l1 = y, l2 = y;
          v1 = x, v2 = x;
          simulated_min = Rcpp::NumericVector::create(y, t);
        }
      } else if (l==0 && a.at(0)!=0) {
        // simulated Bessel layer was [min(x,y)-a[0], max(x,y)+a[0]]
        // but this is the first layer, don't have a[l-1]
        simulated_min = min_sampler(x, y, s, t, std::min(x,y)-a.at(l), std::min(x,y));
        l1 = simulated_min.at(0), l2 = simulated_min.at(0);
        v1 = std::max(x,y)+a.at(l), v2 = std::max(x,y)+a.at(l);
      } else {
        // simulated Bessel layer was [min(x,y)-a[l], max(x,y)+a[l]]
        simulated_min = min_sampler(x, y, s, t, std::min(x,y)-a.at(l), std::min(x,y)-a.at(l-1));
        l1 = simulated_min.at(0), l2 = simulated_min.at(0);
        v1 = std::max(x,y)+a.at(l-1), v2 = std::max(x,y)+a.at(l);
      }
      
      // simulate Bessel bridge conditional on the minimum point at intermediate points
      simulated_BB = min_Bessel_bridge_path_sampler(x, y, s, t, simulated_min.at(0), simulated_min.at(1), 
                                                    bridge_times);
      // check that none of the simulated points are outside of the layer
      // if so, resample Bessel bridge
      while (find_max(simulated_BB) > v2) { 
        simulated_BB = min_Bessel_bridge_path_sampler(x, y, s, t, simulated_min.at(0), simulated_min.at(1),
                                                      bridge_times);
      }
    } else {
      // simulate maximum point
      Rcpp::NumericVector simulated_max;
      if (l==0 && a.at(0)==0) {
        // simulated Bessel layer was [min(x,y), max(x,y)]
        // set maximum to occur at max(x,y)
        if (x < y) {
          // maximum occurs at (y) at time (t)
          l1 = x, l2 = x;
          v1 = y, v2 = y;
          simulated_max = Rcpp::NumericVector::create(y, t);
        } else {
          // maximum occurs at (x) at time (s)
          l1 = y, l2 = y;
          v1 = x, v2 = x;
          simulated_max = Rcpp::NumericVector::create(x, s);
        }
      } else if (l==0 && a.at(0)!=0) {
        // simulated Bessel layer was [min(x,y)-a[0], max(x,y)+a[0]]
        // but this is the first layer, don't have a[l-1]
        simulated_max = max_sampler(x, y, s, t, std::max(x,y), std::max(x,y)+a.at(l));
        l1 = std::min(x,y)-a.at(l), l2 = std::min(x,y)-a.at(l);
        v1 = simulated_max.at(0), v2 = simulated_max.at(0);
      } else {
        // simulated Bessel layer was [min(x,y)-a[l], max(x,y)+a[l]]
        simulated_max = max_sampler(x, y, s, t, std::max(x,y)+a.at(l-1), std::max(x,y)+a.at(l));
        l1 = std::min(x,y)-a.at(l-1), l2 = std::min(x,y)-a.at(l);
        v1 = simulated_max.at(0), v2 = simulated_max.at(0);
      }
      
      // simulate Bessel bridge conditional on the maximum at intermediate points
      simulated_BB = max_Bessel_bridge_path_sampler(x, y, s, t, simulated_max.at(0), simulated_max.at(1), 
                                                    bridge_times);			
      
      // check that none of the simulated points are outside of the layer
      // if so, resample Bessel bridge
      while (find_min(simulated_BB) < l2) {
        simulated_BB = max_Bessel_bridge_path_sampler(x, y, s, t, simulated_max.at(0), simulated_max.at(1),
                                                      bridge_times);
      }
    }
    
    // checking if BB remains in [l1, v1]
    double starting_index_1 = sqrt((t-s)+fabs(v1-l1)*fabs(v1-l1))/(2*fabs(v1-l1));
    int j = ceil(starting_index_1);
    
    Rcpp::NumericVector x_values, y_values, s_values, t_values;
    for (int i=0; i < (simulated_BB.ncol()-1); ++i) {
      x_values.push_back(simulated_BB(0, i));
      y_values.push_back(simulated_BB(0, i+1));
      s_values.push_back(bridge_times.at(i));
      t_values.push_back(bridge_times.at(i+1));
    }
    
    if (u < 0.5) {
      // flip delta coin #1
      if (delta_coin_intervals(x_values, y_values, s_values, t_values, l1, v1, j)) {
        times = bridge_times;
        return simulated_BB;
      } else {
        // checking if BB remains in [l1, v1]
        double starting_index_2 = sqrt((t-s)+fabs(v2-l2)*fabs(v2-l2))/(2*fabs(v2-l2));
        int k = ceil(starting_index_2);
        
        // flip detla coin #2
        if (delta_coin_intervals(x_values, y_values, s_values, t_values, l2, v2, k)) {
          times = bridge_times;
          return simulated_BB;
        } else {
          continue;
        }
      } 
    } else {
      // flip delta coin #1
      if (delta_coin_intervals(x_values, y_values, s_values, t_values, -v1, -l1, j)) {
        times = bridge_times;
        return simulated_BB;
      } else {
        // checking if BB remains in [l1, v1]
        double starting_index_2 = sqrt((t-s)+fabs(v2-l2)*fabs(v2-l2))/(2*fabs(v2-l2));
        int k = ceil(starting_index_2);
        
        if (delta_coin_intervals(x_values, y_values, s_values, t_values, -v2, l2, k)) {
          times = bridge_times;
          return simulated_BB;
        } else {
          continue;
        }
      }
    }
  }	
}
