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

#ifndef __ARSIM_LIMITED_SCHEDULER_K_HPP__
#  define __ARSIM_LIMITED_SCHEDULER_K_HPP__


#include "LimitedController.hpp"
#include "Interval.hpp"

class LimitedControllerK : public LimitedController {

 protected:

  typedef LimitedController parent;
  double getAlphaMin();

 public:

  double calcBandwidth(double sched_err, double start_err);

  double calcBandwidthLimitedAsymm(double sched_err, double start_err);

  LimitedControllerK();
  /** Calculate dependent parameters		*/
  void calcParams();
  /** Check scheduler's parameters consistency	*/
  bool checkParams();
  bool parseArg(int& argc, char **& argv);

  static void usage();
};

#endif
