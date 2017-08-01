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

#ifndef __ARSIM_DBL_TASK_HPP__
#  define __ARSIM_DBL_TASK_HPP__

#include "Task.hpp"

/**
 * Base class for tasks whose computation times are defined
 * w.r.t. two intervals, one inside one another: the default
 * interval is specified through the -c and -C options, while
 * the inner one through the -ci and -Ci options.
 *
 * Values specified on the command line may be overwritten
 * at run-time by subclasses, thus they may be regarded as
 * initial or default values.
 */
class DoubleLimitedTask : public Task {

 protected:

  typedef Task parent;

  double c_min_i;	/** Internal c(k) range minimum		*/
  double c_max_i;	/** Internal c(k) range maximum		*/
  double alpha_i;	/** Minimum c_min_i(k) / c_max_i(k)	*/

  void calcIInterval();	/** Calculate internal range		*/

 public:

  DoubleLimitedTask();
  bool parseArg(int& argc, char **& argv);

  /** Return probable min task instance execution time	*/
  virtual double getMinExecutionTimeI() const { return c_min_i; }
  /** Return probable max task instance execution time	*/
  virtual double getMaxExecutionTimeI() const { return c_max_i; }

  /** Return inf { probable min task instance execution time }	*/
  virtual double getInfMinExecutionTimeI() const { return c_min_i; }
  /** Return sup { probable max task instance execution time }	*/
  virtual double getSupMaxExecutionTimeI() const { return c_max_i; }

  /** Return inf { min / max probable task instance execution time }	*/
  virtual double getInfExecutionTimeIRatio() const;

  static void usage();
};

#endif
