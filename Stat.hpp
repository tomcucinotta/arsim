/*
 * ARSim
 * Copyright (c) 2000-2010 Tommaso Cucinotta
 *
 * This file is part of ARSim.
 *
 * ARSim is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * ARSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with ARSim. If not, see <http://www.gnu.org/licenses/>
 */

#ifndef __ARSIM_STAT_HPP__
#define   __ARSIM_STAT_HPP__

#include "BaseStat.hpp"
#include <vector>
#include "util.hpp"

using namespace std;

class Stat : public BaseStat {
  double x_min, x_max;
  double dx;
  long *x_pmf;		// Number of occurrences in each dx-sized interval
  long x_pmf_size;	// Number of pmf samples: (max-min)/dx
  long num_samples;	// Number of x samples
  double x_sum;		// Exact sum of samples
  double x_sqr_sum;	// Exact sum of square samples
  double x_abs_sum;     // Exact sum of absolute values of samples
  long num_pos;		// Number of positive samples
  double x_pos_sum;	// Exact sum of positive samples
  long num_neg_zero;	// Number of negative or zero samples
  double x_neg_zero_sum; // Exact sum of negative or zero samples

 public:

  typedef BaseStat parent;

   /** Build a Stat object for studying x in [x_min,x_max] at	*
   * steps of dx						*/
  Stat(double x_min, double x_max, double dx);
  /** Specify the number of subintervals to use and infer dx	*/
  Stat(double x_min, double x_max, long x_pmf_size);
  ~Stat();
  /** Feed with next sample					*/
  void addSample(double x);
  /** Get number of samples   */
  virtual long getNumSamples() const { return num_samples; }
  /** Calculate mean from calculated PMF				*/
  double calcPMFMean() const;
  /** Return mean of provided samples				*/
  double getMean() const;
  /** Return mean of strictly positive provided samples		*/
  double getMeanPos() const;
  /** Return mean of negative or zero provided samples		*/
  double getMeanNegZero() const;
  /** Return standard deviation of provided samples             */
  double getDev() const;
  /** Return mean of absolute values of provided samples        */
  double getMeanAbs() const;
  /** Evaluate the distribution function at a single point	*
   * - assume a constant PDF in each dx interval		*/
  double getPDFValue(double x) const;
  /** Return the n-th pmf sample					*/
  double getPMFValue(long n_sample) const;
  /** Sum up PMF values: this is less than 1 iff samples outside	*
   * the [x_min, x_max] interval have been provided		*/
  double sumPMFValues() const;
  /** Return the PMF size					*/
  long getPMFSize() const { return x_pmf_size; }
  /** Return x value getPMFValue(n_sample) refers to		*/
  double getXValue(long n_sample) const;

  template<class T>
    static double getMinPercentile(const T& v, double p, double a, double b);
  template<class I>
    static double getMinPercentile(const I& beg, const I& end, double p, double a, double b);
  template<class T>
    static double getMaxPercentile(const T& v, double p, double a, double b);
  template<class I>
    static double getMaxPercentile(const I& beg, const I& end, double p, double a, double b);
  template<class I>
    static long saturateAtPercentiles(const I& beg, const I& end, double sat_p_min, double sat_p_max);
  template<class I>
    static long saturate(const I& beg, const I& end, double x_min, double x_max);

  /** Clear all accumulated statistics.				*/
  virtual void clear();
};

#include <math.h>

inline void Stat::addSample(double x) {
  BaseStat::addSample(x);
  long n_sample = long(floor((x - x_min) / dx));
  if ((n_sample >= 0) && (n_sample < x_pmf_size))
    x_pmf[n_sample]++;
  num_samples++;
  x_sum += x;
  x_sqr_sum += x*x;
  x_abs_sum += fabs(x);
  if (x > 0.0) {
    num_pos++;
    x_pos_sum += x;
  } else {
    num_neg_zero++;
    x_neg_zero_sum += x;
  }
}

/**
 * Compute the value c that separates the interval [a, b] of values inside the dX vector
 * so that a percentage of values equal to prob_interval remains greater than c
 */
template<typename T>
double Stat::getMinPercentile(const T& v, double p, double a, double b) {
  double mid = (a + b) / 2;
  if (b - a < 0.001)
    return mid;
  int num = 0;
  for (unsigned int i = 0; i < v.size(); ++i)
    if (v[i] >= mid)
      num++;
  double prob = num / (double) v.size();
  if (prob == p)
    return mid;
  else if (prob > p)
    return getMinPercentile(v, p, mid, b);
  else
    return getMinPercentile(v, p, a, mid);
}

/**
 * Compute the value c that separates the interval [a, b] of values inside the dX vector
 * so that a percentage of values equal to prob_interval remains lower than c
 */
template<class I>
double Stat::getMaxPercentile(const I& beg, const I& end, double p, double a, double b) {
  double mid = (a + b) / 2;
  if (b - a < 0.001)
    return mid;
  int num = 0;
  for (I it = beg; it != end; ++it)
    if (*it <= mid)
      num++;
  double prob = num / double(end - beg);
  if (prob == p)
    return mid;
  else if (prob > p)
    return getMaxPercentile(beg, end, p, a, mid);
  else
    return getMaxPercentile(beg, end, p, mid, b);
}

template<class T>
double Stat::getMaxPercentile(const T& v, double p, double a, double b) {
  return getMaxPercentile(v.begin(), v.end(), p, a, b);
}

/**
 * Saturate input at specified bottom and top percentiles.
 */
template<class I>
long Stat::saturateAtPercentiles(const I& beg, const I& end, double sat_p_min, double sat_p_max) {
  double min = *min_element(beg, end);
  double max = *max_element(beg, end);
  double x_min = Stat::getMaxPercentile(beg, end, sat_p_min, min, max);
  double x_max = Stat::getMaxPercentile(beg, end, sat_p_max, min, max);
  return saturate(beg, end, x_min, x_max);
}

template<class I>
long Stat::saturate(const I& beg, const I& end, double x_min, double x_max) {
  ASSERT(x_min <= x_max, "saturate(): Wrong parameters");
  long num_saturated = 0;
  for (I it = beg; it != end; ++it)
    if (*it < x_min) {
      if (it - beg >= 12)
	*it = *(it - 12);
      else
	*it = x_min;
      ++num_saturated;
    } else if (*it > x_max) {
      if (it - beg >= 12)
	*it = *(it - 12);
      else
	*it = x_max;
      ++num_saturated;
    }
  return num_saturated;
}

#endif
