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

#ifndef __ARSIM_PI_SCHEDULER_HPP__
#  define __ARSIM_PI_SCHEDULER_HPP__


#include "Controller.hpp"
#include "Interval.hpp"

/**
 * This class implements a switching linear feedback control law.
 */


class PIController : public Controller {

 protected:

  double u_prev;      //< Last assigned bandwidth
  double eps_prev;	//< Previous value of eps: e(k-1)

  double period;	// Task period (assumed equal to instance deadline)
  double c_min, c_max;	// Minimum and maximum task instance duration

  double z1, z2;	// Poles of the linearized controller
  double eps_target;	// Scheduling error value around which system is stabilized
  double alpha1, beta1;	// Parameters used for linear feedback control when eps < 0
  double alpha2, beta2; // Parameters used for linear feedback control when eps >= 0

  typedef Controller parent;

 public:

  PIController();
  virtual double calcBandwidth(double sched_err, double start_err);
  /** Calculate dependent parameters		*/
  virtual void calcParams();
  /** Check scheduler's parameters consistency	*/
  virtual bool checkParams();
  virtual bool parseArg(int& argc, char **& argv);
  static void usage();
};

#endif
