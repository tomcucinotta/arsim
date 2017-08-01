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

#ifndef __ARSIM_LIMITED_SCHEDULER_R_HPP__
#  define __ARSIM_LIMITED_SCHEDULER_R_HPP__


#include "Controller.hpp"
#include "Interval.hpp"

/** Relaxed limited scheduler: uses a Double Limited Task, but		*
 * calculates invariant and bandwidth based on the internal range.	*
 * Tries to tolerate spikes outside internal range, giving max bandwidth*
 * to steer eps(k) within invariant, when it goes outside		*/

class InvariantController : public Controller {

 protected:

  typedef Controller parent;

  double eps_max;	// Control such that -eps_min < e(k) < eps_max
  double eps_min;	// Control such that -eps_min < e(k) < eps_max
  double coeff_pos;	// Percentage of the minimum invariant that remains positive
  double c_min, c_max;	// Minimum and maximum task instance duration

  Stat eps_inv_stat;	// Experimental probability of staying into the invariant

  /**
   * Event-based statistics for number of consecutive jobs for which the
   * sched err remains outside the invariant set
   */
  Stat rsteps_inv_stat;

  /**
   * Partial number of consecutive jobs for which the sched err remains
   * outside the invariant set
   */
  int rsteps_inv;

  /** File where to dump rsteps stats to	*/
  char *rsteps_inv_fname;

  /** Stat of prediction range widths		*/
  Stat range_width_stat;
  /** Stat of prediction range alphas		*/
  Stat range_alpha_stat;

  /** Turns on strict checking on invariant conditions */
  bool strict_checks;

  Interval calcBandwidthRange(double sched_err, double eps);
  Interval calcBandwidthRangeNoUpdate(double start_err) const;

 public:

  double calcBandwidth(double sched_err, double start_err);

  double calcBandwidthLimitedAsymm(double sched_err, double start_err);

  InvariantController();
  /** Calculate dependent parameters		*/
  void calcParams();
  /** Check scheduler's parameters consistency	*/
  bool checkParams();
  bool parseArg(int& argc, char **& argv);

  double getUsedMaxError() const { return eps_max; }
  double getUsedMinError() const { return eps_min; }

  /** Calculate single invariant */
  static void calcInvariant(double period, double alpha, double coeff_pos, double& eps_min, double& eps_max);

  /** Compute invariant-based unconstrainted bandwidth interval (may have min,max -> infinity) */
  static Interval calcBwRange(
    double start_err, double period,
    double c_min, double c_max,
    double eps_min, double eps_max
  );

  static void usage();

  void dumpStats();

};

#endif
