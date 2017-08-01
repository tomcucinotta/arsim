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

#ifndef __ARSIM_DOUBLE_LIMITED_SCHEDULER_HPP__
#  define __ARSIM_DOUBLE_LIMITED_SCHEDULER_HPP__


#include "InvariantController.hpp"
#include "Interval.hpp"
#include "Stat.hpp"
#include <deque>
#include <vector>

class DoubleInvariantController : public InvariantController {

 private:

  typedef InvariantController parent;

  double c_min_i, c_max_i;	     /**< Internal c(k) excursion range	*/
  double eps_min_i, eps_max_i;	 /**< Internal e(k) excursion range	*/
  double coeff_pos_i;		         /**< Percentage of internal range width that remains positive */
  std::deque<double> q;
  double sample_size;		         /**< For estimating moveable mean and [c_m(k), c_M(k)] */

  int n_eps_i;      /**< Track # of times e(k) is in internal range		*/
  int n_eps_o;      /**< Track # of times e(k) is out of internal range	*/
  Stat stat_o_len;
  int curr_o_len;	  /**< Calc length of current chain outside internal range	*/

  /** Calculate mean of dist_o_len distribution */
  double meanDistOut();

  Interval bw_range;    //< Last computed bandwidth range (intersection)
  Interval bw_range_i;  //< Last computed bandwidth range (internal only)

  bool best_effort;     //< If external invariant does not need to be guaranteed

 public:

  DoubleInvariantController();

  virtual double calcBandwidth(double sched_err, double start_err);
  Interval calcBwRangeI(double start_err);

  /** Calculate dependent parameters		*/
  virtual void calcParams();
  void calcParams1();
  /** Check scheduler's parameters consistency	*/
  virtual bool checkParams();
  bool checkParams1();
  virtual bool parseArg(int& argc, char **& argv);

  static void usage();

  /** Forecast internal variability range for c(k) */
  Interval getTaskExpIntervalI();

  void updateStatistics();
  void showStatistics();

  inline Interval getBwRange() const { return bw_range; }
  inline Interval getBwRangeI() const { return bw_range_i; }
  double getTaskInfExecutionTimeIRatio();

  inline bool isBestEffort() const { return best_effort; }

  virtual void setLastJobExecTime(double c);
};

#endif
