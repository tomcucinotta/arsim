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

#ifndef __ARSIM_STAT_INTERFACE_HPP__
#define   __ARSIM_STAT_INTERFACE_HPP__

#include <values.h>
#include <algorithm>

class BaseStat {

protected:

  double min_val;       //< Minimum experimented value
  double max_val;       //< Maximum experimented value
  BaseStat() { clear(); }
  virtual void addSample(double value) {
    min_val = std::min(min_val, value);
    max_val = std::max(max_val, value);
  }

public:

  /** Calculate mean from calculated PMF			*/
  virtual double calcPMFMean() const = 0;
  /** Return mean of provided samples				*/
  virtual double getMean() const = 0;
  /** Return mean of positive provided samples			*/
  virtual double getMeanPos() const = 0;
  /** Return mean of negative or zero provided samples		*/
  virtual double getMeanNegZero() const = 0;
  /** Return standard deviation of provided samples		*/
  virtual double getDev() const = 0;
  /** Return mean of absolute values of provided samples        */
  virtual double getMeanAbs() const = 0;

  /** Return min of provided samples                            */
  virtual double getMin() const { return min_val; }
  /** Return max of provided samples                            */
  virtual double getMax() const { return max_val; }

  /** Evaluate the distribution function at a single point	*
   * - assume a constant PDF in each dx interval		*/
  virtual double getPDFValue(double x) const = 0;
  /** Return the n-th pmf sample				*/
  virtual double getPMFValue(long n_sample) const = 0;
  /** Sum up PMF values: this is less than 1 iff samples outside *
   * the [x_min, x_max] interval have been provided		*/
  virtual double sumPMFValues() const = 0;
  /** Return the PMF size					*/
  virtual long getPMFSize() const  = 0;
  /** Return x value getPMFValue(n_sample) refers to		*/
  virtual double getXValue(long n_sample) const = 0;

  /** Approximate the required percentile, in the range 0.0-0.1,
   ** by using the information stored within the cumulated PMF
   ** intervals.						*/
  virtual double getPMFPercentile(double p) const;

  /** Clear all accumulated statistics.
   **
   ** After this call, the internal state is the same as after
   ** construction.
   **
   ** @note
   ** The clear() method is easier to use and more efficient
   ** than destroying the class istance and building a new one.
   **/
  virtual void clear() {
    min_val = MAXDOUBLE;
    max_val = -MAXDOUBLE;
  }

  virtual long getNumSamples() const = 0;

  /** Dumps statistic info contained in the TimeStat
   ** object into the specified file
   **/
  void dumpStat(const char *fname, const char *var_name,
		bool dump_as_pmf, const char *comment = "No comment");

  /** Virtual destructor */
  virtual ~BaseStat() { };
};

#endif
