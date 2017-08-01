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

#ifndef __ARSIM_LIMITED_SCHEDULER_HPP__
#  define __ARSIM_LIMITED_SCHEDULER_HPP__


#include "Controller.hpp"
#include "Interval.hpp"

/** Limited controller */
class LimitedController : public Controller {

 protected:

  double eps_max;	//< Control such that -eps_min < e(k) < eps_max
  double eps_min;	//< Control such that -eps_min < e(k) < eps_max
  double coeff_pos;	//< Percentage of the minimum invariant that remains positive
  double period;	//< Task period (assumed equal to instance deadline)
  double c_min, c_max;	//< Minimum and maximum task instance duration
  double gamma;		//< Reduction factor in exponential scheduler

  typedef double (LimitedController::*CalcBandwidthMethod)(double eps);
  typedef Controller parent;

  CalcBandwidthMethod cbw_method;

  bool strict_checks;   //< Turns on strict checking on external invariant conditions */

 public:

  double calcBandwidth(double sched_err, double start_err);

  Interval calcUIntervalLimitedSymm(double eps, double c_min, double c_max, double eps_max);
  double calcBandwidthLimitedSymm(double eps);

  static Interval calcUIntervalLimitedAsymm(
    double eps, double period,
    double c_min, double c_max,
    double eps_min, double eps_max
  );
  double calcBandwidthLimitedAsymm(double eps);
  double calcBandwidthLimitedAsymm_k(double eps);
  static Interval calcBwRange(
    double start_err, double period,
    double c_min, double c_max,
    double eps_min, double eps_max
  );

  Interval calcBwRange(double start_err);

  Interval calcUIntervalReduced(double eps, double c_min, double c_max, double gamma);
  double calcBandwidthReduced(double eps);

  Interval calcUIntervalYAsymm(double eps);
  double calcBandwidthLimitedEY(double eps);

  LimitedController();
  /** Calculate dependent parameters		*/
  void calcParams();
  /** Check scheduler's parameters consistency	*/
  bool checkParams();
  bool parseArg(int& argc, char **& argv);

  double getUsedMaxError() const { return eps_max; }
  double getUsedMinError() const { return eps_min; }

  void setMethod(CalcBandwidthMethod cbmeth);
  /** Calculate single invariant */
  static void calcInvariant(double period, double alpha, double coeff_pos, double& eps_min, double& eps_max);

  static void usage();
};

#endif
