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

#ifndef __ARSIM_TIME_STAT_HPP__
#define   __ARSIM_TIME_STAT_HPP__

#include "BaseStat.hpp"

class TimeStat : public BaseStat {
  double x_min, x_max;
  double dx;
  double *x_pmf;	// Temporally weighted occurrences in each dx-sized interval
  long x_pmf_size;	// Number of pmf samples: (max-min)/dx
  long num_samples;	// Number of x samples
  double x_sum;		// Temporally weighted sum of samples
  double x_sqr_sum;	// Temporally weighted sum of square samples
  double x_abs_sum;     // Temporally weighted sum of absolute samples
  double x_sum_pos;	// Temporally weighted sum of positive samples
  double x_sum_neg_zero; // Temporally weighted sum of negative or zero samples
  double t_sum_pos;	// Sum of temporal duration of positive samples
  double t_sum_neg_zero; // Sum of temporal duration of negative or zero samples
  double prev_x;	// Last sample
  double prev_t;	// Time of insertion of last sample
  double orig_t;        // Time of start of statistics accumulation

 public:

  typedef BaseStat parent;

   /** Build a TimeStat object for studying the temporal evolution
   * of x(t) in [x_min,x_max] at steps of dx			*/
  TimeStat(double x_min, double x_max, double dx, double now = 0.0);
  /** Specify the number of subintervals to use and infer dx	*/
  TimeStat(double x_min, double x_max, long x_pmf_size, double now = 0.0);

  ~TimeStat();
  /** Feed with next sample at time t				*/
  void addSample(double x, double t);
  /** Get number of samples   */
  virtual long getNumSamples() const { return num_samples; }
  /** Calculate mean from calculated PMF			*/
  double calcPMFMean() const;
  /** Return mean of provided samples				*/
  double getMean() const;
  /** Return mean of positive provided samples			*/
  double getMeanPos() const;
  /** Return mean of negative or zero provided samples		*/
  double getMeanNegZero() const;
  /** Return standard deviation of provided samples		*/
  double getDev() const;
  /** Return mean of absolute values of provided samples        */
  double getMeanAbs() const;
  /** Evaluate the distribution function at a single point	*
   * - assume a constant PDF in each dx interval		*/
  double getPDFValue(double x) const;
  /** Return the n-th pmf sample				*/
  double getPMFValue(long n_sample) const;
  /** Sum up PMF values: this is less than 1 iff samples outside *
   * the [x_min, x_max] interval have been provided		*/
  double sumPMFValues() const;
  /** Return the PMF size					*/
  long getPMFSize() const { return x_pmf_size; }
  /** Return x value getPMFValue(n_sample) refers to		*/
  double getXValue(long n_sample) const;

  /** Clear all accumulated statistics.				*/
  virtual void clear(double now);
};

#include <math.h>

inline void TimeStat::addSample(double x, double t) {
  BaseStat::addSample(x);
  /* Use previous sample, that was kept for a time equal to t-prev_t	*/
  long n_sample = long(floor((prev_x - x_min) / dx));
  double w = t - prev_t;

  if (n_sample < 0)
    n_sample = 0;
  else if (n_sample >= x_pmf_size)
    n_sample = x_pmf_size - 1;

  x_pmf[n_sample] += w;
  num_samples++;
  x_sum += prev_x * w;
  x_sqr_sum += prev_x * prev_x * w;
  x_abs_sum += fabs(prev_x) * w;
  if (prev_x == 0.0) {
    x_sum_pos += prev_x * w;
    t_sum_pos += w;
  } else {
    x_sum_neg_zero += prev_x * w;
    t_sum_neg_zero += w;
  }
  /* Replace prev_x and prev_t with new values			*/
  prev_x = x;
  prev_t = t;
}

#endif
