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

#ifndef __ARSIM_SDB_SCHEDULER_R_HPP__
#  define __ARSIM_SDB_SCHEDULER_R_HPP__


#include "Controller.hpp"
#include "Interval.hpp"

/** Stochastic Dead Beat Controller. Always assigns bandwidth	*
 * value achieving an expected next scheduling equal to zero.	*/

class SDBController : public Controller {

 protected:

  typedef Controller parent;

  /** Target scheduling error value (defaults to zero)	*/
  double target_eps;

 public:

  virtual double calcBandwidth(double sched_err, double start_err);

  SDBController();
  virtual bool parseArg(int& argc, char **& argv);
  static void usage();
};

#endif
