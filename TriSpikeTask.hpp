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

#ifndef __ARSIM_TRI_SPIKE_TASK_HPP__
#  define __ARSIM_TRI_SPIKE_TASK_HPP__

#include "Task.hpp"
#include "SpikeTask.hpp"

class TriSpikeTask : public SpikeTask {

 private:

  typedef SpikeTask parent;

  double c_mean_last, dc_last, dc_abs; /** Moving mean of c(k)	*/

  int burst_countdown;	/** Support brusty c_max			*/

 public:

  TriSpikeTask();
  /** Return next task instance time c(k) */
  virtual double generateInstance();
  virtual bool parseArg(int& argc, char **& argv);
  /** Return min task instance execution time	*/
  virtual double getMinExecutionTimeI() const;
  /** Return max task instance execution time	*/
  virtual double getMaxExecutionTimeI() const;

  /** Return inf { probable min task instance execution time }	*/
  virtual double getInfMinExecutionTimeI() const { return c_min_i; }
  /** Return sup { probable max task instance execution time }	*/
  virtual double getSupMaxExecutionTimeI() const { return c_max_i; }

  /** Return inf { min / max probable task instance execution time }	*/
  virtual double getInfExecutionTimeIRatio() const;

  static void usage();
};

#endif
