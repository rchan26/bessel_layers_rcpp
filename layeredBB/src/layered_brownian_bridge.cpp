#include "../inc/layered_brownian_bridge.hpp"
#include "../inc/brownian_bridge_min.hpp"
#include "../inc/inverse_gauss.hpp"
#include "../inc/cauchy_sums.hpp"
#include "../inc/coins_cauchy.hpp"
#include "../inc/bessel_layer_sim.hpp"

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
                                            const Rcpp::NumericVector &a, int l,
                                            const Rcpp::NumericVector &times)
{
  // layered Brownian bridge sampler
  // simulate from uniform random variable
  double u = Rcpp::runif(1, 0.0, 1.0)[0];
  int count = 1;
  // we minus 1 from the Bessel layer (l), since indicies start from 0 in C++ but start from 1 in R
  l -= 1;
  
  while (true) {
    // add in line to abort C++ if user has pressed Ctrl/Cmd+C or Escape in R
    Rcpp::checkUserInterrupt();
    
    // declare variables required
    double l1, l2, v1, v2;
    Rcpp::NumericMatrix simulated_BB;
    // Rcpp::NumericVector bridge_times = times;
    
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
          simulated_min = Rcpp::NumericVector::create(Named("min", x), Named("tau", s));
        } else {
          // minimum occurs at (y) at time (t)
          l1 = y, l2 = y;
          v1 = x, v2 = x;
          simulated_min = Rcpp::NumericVector::create(Named("min", y), Named("tau", t));
        }
      } else if (l==0 && a.at(0)!=0) {
        // simulated Bessel layer was [min(x,y)-a[0], max(x,y)+a[0]]
        // but this is the first layer, don't have a[l-1]
        simulated_min = min_sampler(x, y, s, t, std::min(x,y)-a.at(l), std::min(x,y));
        l1 = simulated_min["min"], l2 = simulated_min["min"];
        v1 = std::max(x,y)+a.at(l), v2 = std::max(x,y)+a.at(l);
      } else {
        // simulated Bessel layer was [min(x,y)-a[l], max(x,y)+a[l]]
        simulated_min = min_sampler(x, y, s, t, std::min(x,y)-a.at(l), std::min(x,y)-a.at(l-1));
        l1 = simulated_min["min"], l2 = simulated_min["min"];
        v1 = std::max(x,y)+a.at(l-1), v2 = std::max(x,y)+a.at(l);
      }
      
      // Rcout << "&&&&&&&&&&&&&&&&&&&&&& Simulated a minimum &&&&&&&&&&&&&&&&&&&&&& \n";
      // Rcout << "l1: " << l1 << "\n";
      // Rcout << "l2: " << l2 << "\n";
      // Rcout << "v1: " << v1 << "\n";
      // Rcout << "v2: " << v2 << "\n";
      
      // simulate Bessel bridge conditional on the minimum point at intermediate points
      simulated_BB = min_Bessel_bridge_path_sampler(x, y, s, t, simulated_min["min"], simulated_min["tau"], 
                                                    times);
      
      // Rcout << "First sampled BB: ";
      // for (auto const &element: simulated_BB(0, _)) {
      //   Rcout << element << " ";
      // }
      // Rcout << "\n";
      // Rcout << "First sampled BB (times): ";
      // for (auto const &element: simulated_BB(1, _)) {
      //   Rcout << element << " ";
      // }
      // Rcout << "\n";
      
      // check that none of the simulated points are outside of the layer
      // if so, resample Bessel bridge
      // int resample_count = 0;
      while (find_max(simulated_BB(0, _)) > v2) { 
        // resample_count += 1;
        simulated_BB = min_Bessel_bridge_path_sampler(x, y, s, t, simulated_min["min"], simulated_min["tau"],
                                                      times);
        // Rcout << "******************** Resampling BB ******************** \n";
        // Rcout << "££££££££££££££££££££££@@@@@@@@ COUNT = " << resample_count << " @@@@@@@@££££££££££££££££££££££ \n";
        // if (resample_count == 1000) {
        //   Rcout << "x: " << x << "\n";
        //   Rcout << "y: " << y << "\n";
        //   Rcout << "s: " << s << "\n";
        //   Rcout << "t: " << t << "\n";
        //   Rcout << "l1: " << l1 << "\n";
        //   Rcout << "l2: " << l2 << "\n";
        //   Rcout << "v1: " << v1 << "\n";
        //   Rcout << "v2: " << v2 << "\n";
        //   Rcout << "min: " << simulated_min.at(0) << "\n";
        //   Rcout << "tau: " << simulated_min.at(1) << "\n";
        //   Rcout << "a: ";
        //   for (auto const &element: a) {
        //     Rcout << element << " ";
        //   }
        //   Rcout << "l: " << l << "\n";
        //   Rcout << "times: ";
        //   for (auto const &element: times) {
        //     Rcout << element << " ";
        //   }
        //   Rcout << "\n";
        //   stop("resample_count = 1000");
        // }
        // Rcout << "resample_count = " << resample_count << "\n";
        // Rcout << "x: " << x << "\n";
        // Rcout << "y: " << y << "\n";
        // Rcout << "s: " << s << "\n";
        // Rcout << "t: " << t << "\n";
        // Rcout << "l1: " << l1 << "\n";
        // Rcout << "l2: " << l2 << "\n";
        // Rcout << "v1: " << v1 << "\n";
        // Rcout << "v2: " << v2 << "\n";
        // Rcout << "Resampled BB: ";
        // for (auto const &element: simulated_BB(0, _)) {
        //   Rcout << element << " ";
        // }
        // Rcout << "\n";
        // Rcout << "Resampled BB (times): ";
        // for (auto const &element: simulated_BB(1, _)) {
        //   Rcout << element << " ";
        // }
        // Rcout << "\n";
        // Rcout << "max(simulated_BB(0, _)): " << find_max(simulated_BB(0,_)) << "\n";
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
          simulated_max = Rcpp::NumericVector::create(Named("max", y), Named("tau", t));
        } else {
          // maximum occurs at (x) at time (s)
          l1 = y, l2 = y;
          v1 = x, v2 = x;
          simulated_max = Rcpp::NumericVector::create(Named("max", x), Named("tau", s));
        }
      } else if (l==0 && a.at(0)!=0) {
        // simulated Bessel layer was [min(x,y)-a[0], max(x,y)+a[0]]
        // but this is the first layer, don't have a[l-1]
        simulated_max = max_sampler(x, y, s, t, std::max(x,y), std::max(x,y)+a.at(l));
        l1 = std::min(x,y)-a.at(l), l2 = std::min(x,y)-a.at(l);
        v1 = simulated_max["max"], v2 = simulated_max["max"];
      } else {
        // simulated Bessel layer was [min(x,y)-a[l], max(x,y)+a[l]]
        simulated_max = max_sampler(x, y, s, t, std::max(x,y)+a.at(l-1), std::max(x,y)+a.at(l));
        l1 = std::min(x,y)-a.at(l-1), l2 = std::min(x,y)-a.at(l);
        v1 = simulated_max["max"], v2 = simulated_max["max"];
      }
      
      // Rcout << "^^^^^^^^^^^^^^^^^^^^^^ Simulated a maximum ^^^^^^^^^^^^^^^^^^^^^^\n";
      // Rcout << "l1: " << l1 << "\n";
      // Rcout << "l2: " << l2 << "\n";
      // Rcout << "v1: " << v1 << "\n";
      // Rcout << "v2: " << v2 << "\n";
      
      // simulate Bessel bridge conditional on the maximum at intermediate points
      simulated_BB = max_Bessel_bridge_path_sampler(x, y, s, t, simulated_max["max"], simulated_max["tau"], 
                                                    times);
      
      // Rcout << "First sampled BB: ";
      // for (auto const &element: simulated_BB(0, _)) {
      //   Rcout << element << " ";
      // }
      // Rcout << "\n";
      // Rcout << "First sampled BB (times): ";
      // for (auto const &element: simulated_BB(1, _)) {
      //   Rcout << element << " ";
      // }
      // Rcout << "\n";
      
      // check that none of the simulated points are outside of the layer
      // if so, resample Bessel bridge
      // int resample_count = 0;
      while (find_min(simulated_BB(0, _)) < l2) {
        // resample_count += 1;
        simulated_BB = max_Bessel_bridge_path_sampler(x, y, s, t, simulated_max["max"], simulated_max["tau"],
                                                      times);
        // Rcout << "******************** Resampling BB ********************\n";
        // Rcout << "££££££££££££££££££££££@@@@@@@@ COUNT = " << resample_count << " @@@@@@@@££££££££££££££££££££££ \n";
        // if (resample_count == 1000) {
        //   Rcout << "x: " << x << "\n";
        //   Rcout << "y: " << y << "\n";
        //   Rcout << "s: " << s << "\n";
        //   Rcout << "t: " << t << "\n";
        //   Rcout << "l1: " << l1 << "\n";
        //   Rcout << "l2: " << l2 << "\n";
        //   Rcout << "v1: " << v1 << "\n";
        //   Rcout << "v2: " << v2 << "\n";
        //   Rcout << "max: " << simulated_max.at(0) << "\n";
        //   Rcout << "tau: " << simulated_max.at(1) << "\n";
        //   Rcout << "a: ";
        //   for (auto const &element: a) {
        //     Rcout << element << " ";
        //   }
        //   Rcout << "l: " << l << "\n";
        //   Rcout << "times: ";
        //   for (auto const &element: times) {
        //     Rcout << element << " ";
        //   }
        //   Rcout << "\n";
        //   stop("resample_count = 1000");
        // }
        // Rcout << "resample_count = " << resample_count << "\n";
        // Rcout << "x: " << x << "\n";
        // Rcout << "y: " << y << "\n";
        // Rcout << "s: " << s << "\n";
        // Rcout << "t: " << t << "\n";
        // Rcout << "l1: " << l1 << "\n";
        // Rcout << "l2: " << l2 << "\n";
        // Rcout << "v1: " << v1 << "\n";
        // Rcout << "v2: " << v2 << "\n";
        // Rcout << "Resampled BB: ";
        // for (auto const &element: simulated_BB(0, _)) {
        //   Rcout << element << " ";
        // }
        // Rcout << "\n";
        // Rcout << "Resampled BB (times): ";
        // for (auto const &element: simulated_BB(1, _)) {
        //   Rcout << element << " ";
        // }
        // Rcout << "\n";
        // Rcout << "min(simulated_BB(0, _)): " << find_min(simulated_BB(0,_)) << "\n";
      }
    }
    
    // checking if BB remains in [l1, v1]
    double starting_index_1 = sqrt((t-s)+fabs(v1-l1)*fabs(v1-l1))/(2*fabs(v1-l1));
    int j = ceil(starting_index_1);
    
    // Rcout << "starting_index_1: " << starting_index_1 << "\n";
    // Rcout << "j: " << j << "\n";
    
    Rcpp::NumericVector x_values, y_values, s_values, t_values;
    for (int i=0; i < (simulated_BB.ncol()-1); ++i) {
      // Rcout << "Assigning values into x_values, y_values, s_values, t_values \n";
      x_values.push_back(simulated_BB(0, i));
      y_values.push_back(simulated_BB(0, i+1));
      s_values.push_back(simulated_BB(1, i));
      t_values.push_back(simulated_BB(1, i+1));
    }
    
    // Rcout << "x_values: ";
    // for (auto const &element: x_values) {
    //   Rcout << element << " ";
    // }
    // Rcout << "\n";
    // Rcout << "y_values: ";
    // for (auto const &element: y_values) {
    //   Rcout << element << " ";
    // }
    // Rcout << "\n";
    // Rcout << "s_values: ";
    // for (auto const &element: s_values) {
    //   Rcout << element << " ";
    // }
    // Rcout << "\n";
    // Rcout << "t_values: ";
    // for (auto const &element: t_values) {
    //   Rcout << element << " ";
    // }
    // Rcout << "\n";
    
    if (u < 0.5) {
      // flip delta coin #1
      if (delta_coin_intervals(x_values, y_values, s_values, t_values, l1, v1, j)) {
        // Rcout << "Done: Simulated minimum and accepted delta coin #1 \n";
        // times = bridge_times;
        return simulated_BB;
      } else {
        // Rcout << "Simulated minimum and rejected delta coin #1 \n";
        // checking if BB remains in [l1, v1]
        double starting_index_2 = sqrt((t-s)+fabs(v2-l2)*fabs(v2-l2))/(2*fabs(v2-l2));
        int k = ceil(starting_index_2);
        
        // Rcout << "starting_index_2: " << starting_index_2 << "\n";
        // Rcout << "k: " << k << "\n";
        
        // flip detla coin #2
        if (delta_coin_intervals(x_values, y_values, s_values, t_values, l2, v2, k)) {
          // Rcout << "Done: Simulated minimum and accepted delta coin #2 \n";
          // times = bridge_times;
          return simulated_BB;
        } else {
          // Rcout << "Simulated minimum and rejected delta coin #2 \n";
          // Rcout << "$$$$$$$$$$$$$$$$$$$$ Starting again $$$$$$$$$$$$$$$$$$$$ \n";
          // Rcout << "times: ";
          // for (auto const &element: times) {
          //   Rcout << element << " ";
          // }
          // Rcout << "\n";
          // count += 1;
          // Rcout << "££££££££££££££££££££££@@@@@@@@@@@@@@@@@@ COUNT = " << count << " @@@@@@@@@@@@@@@@@@££££££££££££££££££££££ \n";
          // Rcout << "a: ";
          // for (auto const &element: a) {
          //   Rcout << element << " ";
          // }
          // Rcout << "\n";
          // Rcout << "l: " << l << "\n";
          if (count > 1000) {
            stop("count>10000");
          }
          continue;
        }
      } 
    } else {
      // flip delta coin #1
      if (delta_coin_intervals(-x_values, -y_values, s_values, t_values, -v1, -l1, j)) {
        // Rcout << "Done: Simulated maximum and accepted delta coin #1 \n";
        // times = bridge_times;
        return simulated_BB;
      } else {
        // Rcout << "Simulated maximum and rejected delta coin #1 \n";
        // checking if BB remains in [l1, v1]
        double starting_index_2 = sqrt((t-s)+fabs(v2-l2)*fabs(v2-l2))/(2*fabs(v2-l2));
        int k = ceil(starting_index_2);
        
        // Rcout << "starting_index_2: " << starting_index_2 << "\n";
        // Rcout << "k: " << k << "\n";
        
        if (delta_coin_intervals(-x_values, -y_values, s_values, t_values, -v2, -l2, k)) {
          // Rcout << "Done: simulated maximum and accepted delta coin #2 \n";
          // times = bridge_times;
          return simulated_BB;
        } else {
          // Rcout << "Simulated maximum and rejected delta coin #2 \n";
          // Rcout << "$$$$$$$$$$$$$$$$$$$$ Starting again $$$$$$$$$$$$$$$$$$$$ \n";
          // Rcout << "times: ";
          // for (auto const &element: times) {
          //   Rcout << element << " ";
          // }
          // Rcout << "\n";
          // count += 1;
          // Rcout << "££££££££££££££££££££££@@@@@@@@@@@@@@@@@@ COUNT = " << count << " @@@@@@@@@@@@@@@@@@££££££££££££££££££££££ \n";
          // 
          // Rcout << "a: ";
          // for (auto const &element: a) {
          //   Rcout << element << " ";
          // }
          // Rcout << "\n";
          // Rcout << "l: " << l << "\n";
          if (count > 1000) {
            stop("count>10000");
          }
          continue;
        }
      }
    }
  }	
}






// // [[Rcpp::export]]
// Rcpp::NumericMatrix layered_brownian_bridge(const double &x, const double &y,
//                                             const double &s, const double &t,
//                                             const Rcpp::NumericVector &a, int l,
//                                             const Rcpp::NumericVector &times)
// {
//   // layered Brownian bridge sampler
//   // simulate from uniform random variable
//   double u = Rcpp::runif(1, 0.0, 1.0)[0];
//   // we minus 1 from the Bessel layer (l), since indicies start from 0 in C++ but start from 1 in R
//   l -= 1;
//   
//   while (true) {
//     // add in line to abort C++ if user has pressed Ctrl/Cmd+C or Escape in R
//     Rcpp::checkUserInterrupt();
//     
//     // declare variables required
//     double l1, l2, v1, v2;
//     Rcpp::NumericMatrix simulated_BB;
//     // Rcpp::NumericVector bridge_times = times;
//     
//     if (u < 0.5) {
//       // simulate minimum point
//       Rcpp::NumericVector simulated_min;
//       
//       // setting l1, l2, v1, v2 values in the different cases
//       // simulating minimum points if needed
//       if (l==0 && a.at(0)==0) {
//         // simulated Bessel layer was [min(x,y), max(x,y)]
//         // set minimum to occur at min(x,y)
//         if (x < y) {
//           // mimimum occurs at (x) at time (s)
//           l1 = x, l2 = x;
//           v1 = y, v2 = y;
//           simulated_min = Rcpp::NumericVector::create(Named("min", x), Named("tau", s));
//         } else {
//           // minimum occurs at (y) at time (t)
//           l1 = y, l2 = y;
//           v1 = x, v2 = x;
//           simulated_min = Rcpp::NumericVector::create(Named("min", y), Named("tau", t));
//         }
//       } else if (l==0 && a.at(0)!=0) {
//         // simulated Bessel layer was [min(x,y)-a[0], max(x,y)+a[0]]
//         // but this is the first layer, don't have a[l-1]
//         simulated_min = min_sampler(x, y, s, t, std::min(x,y)-a.at(l), std::min(x,y));
//         l1 = simulated_min["min"], l2 = simulated_min["min"];
//         v1 = std::max(x,y)+a.at(l), v2 = std::max(x,y)+a.at(l);
//       } else {
//         // simulated Bessel layer was [min(x,y)-a[l], max(x,y)+a[l]]
//         simulated_min = min_sampler(x, y, s, t, std::min(x,y)-a.at(l), std::min(x,y)-a.at(l-1));
//         l1 = simulated_min["min"], l2 = simulated_min["min"];
//         v1 = std::max(x,y)+a.at(l-1), v2 = std::max(x,y)+a.at(l);
//       }
//       
//       // simulate Bessel bridge conditional on the minimum point at intermediate points
//       simulated_BB = min_Bessel_bridge_path_sampler(x, y, s, t, simulated_min["min"], simulated_min["tau"], 
//                                                     times);
//       
//       // check that none of the simulated points are outside of the layer
//       // if so, resample Bessel bridge
//       while (find_max(simulated_BB(0, _)) > v2) { 
//         simulated_BB = min_Bessel_bridge_path_sampler(x, y, s, t, simulated_min["min"], simulated_min["tau"],
//                                                       times);
//       }
//     } else {
//       // simulate maximum point
//       Rcpp::NumericVector simulated_max;
//       if (l==0 && a.at(0)==0) {
//         // simulated Bessel layer was [min(x,y), max(x,y)]
//         // set maximum to occur at max(x,y)
//         if (x < y) {
//           // maximum occurs at (y) at time (t)
//           l1 = x, l2 = x;
//           v1 = y, v2 = y;
//           simulated_max = Rcpp::NumericVector::create(Named("max", y), Named("tau", t));
//         } else {
//           // maximum occurs at (x) at time (s)
//           l1 = y, l2 = y;
//           v1 = x, v2 = x;
//           simulated_max = Rcpp::NumericVector::create(Named("max", x), Named("tau", s));
//         }
//       } else if (l==0 && a.at(0)!=0) {
//         // simulated Bessel layer was [min(x,y)-a[0], max(x,y)+a[0]]
//         // but this is the first layer, don't have a[l-1]
//         simulated_max = max_sampler(x, y, s, t, std::max(x,y), std::max(x,y)+a.at(l));
//         l1 = std::min(x,y)-a.at(l), l2 = std::min(x,y)-a.at(l);
//         v1 = simulated_max["max"], v2 = simulated_max["max"];
//       } else {
//         // simulated Bessel layer was [min(x,y)-a[l], max(x,y)+a[l]]
//         simulated_max = max_sampler(x, y, s, t, std::max(x,y)+a.at(l-1), std::max(x,y)+a.at(l));
//         l1 = std::min(x,y)-a.at(l-1), l2 = std::min(x,y)-a.at(l);
//         v1 = simulated_max["max"], v2 = simulated_max["max"];
//       }
//       
//       // simulate Bessel bridge conditional on the maximum at intermediate points
//       simulated_BB = max_Bessel_bridge_path_sampler(x, y, s, t, simulated_max["max"], simulated_max["tau"], 
//                                                     times);
//       
//       // check that none of the simulated points are outside of the layer
//       // if so, resample Bessel bridge
//       while (find_min(simulated_BB(0, _)) < l2) {
//         simulated_BB = max_Bessel_bridge_path_sampler(x, y, s, t, simulated_max["max"], simulated_max["tau"],
//                                                       times);
//       }
//     }
//     
//     // checking if BB remains in [l1, v1]
//     double starting_index_1 = sqrt((t-s)+fabs(v1-l1)*fabs(v1-l1))/(2*fabs(v1-l1));
//     int j = ceil(starting_index_1);
//     
//     Rcpp::NumericVector x_values, y_values, s_values, t_values;
//     for (int i=0; i < (simulated_BB.ncol()-1); ++i) {
//       x_values.push_back(simulated_BB(0, i));
//       y_values.push_back(simulated_BB(0, i+1));
//       s_values.push_back(simulated_BB(1, i));
//       t_values.push_back(simulated_BB(1, i+1));
//     }
//     
//     if (u < 0.5) {
//       // flip delta coin #1
//       if (delta_coin_intervals(x_values, y_values, s_values, t_values, l1, v1, j)) {
//         return simulated_BB;
//       } else {
//         double starting_index_2 = sqrt((t-s)+fabs(v2-l2)*fabs(v2-l2))/(2*fabs(v2-l2));
//         int k = ceil(starting_index_2);
//         
//         // flip detla coin #2
//         if (delta_coin_intervals(x_values, y_values, s_values, t_values, l2, v2, k)) {
//           return simulated_BB;
//         } else {
//           continue;
//         }
//       } 
//     } else {
//       // flip delta coin #1
//       if (delta_coin_intervals(-x_values, -y_values, s_values, t_values, -v1, -l1, j)) {
//         return simulated_BB;
//       } else {
//         // checking if BB remains in [l1, v1]
//         double starting_index_2 = sqrt((t-s)+fabs(v2-l2)*fabs(v2-l2))/(2*fabs(v2-l2));
//         int k = ceil(starting_index_2);
//         
//         if (delta_coin_intervals(-x_values, -y_values, s_values, t_values, -v2, -l2, k)) {
//           return simulated_BB;
//         } else {
//           continue;
//         }
//       }
//     }
//   }	
// }
